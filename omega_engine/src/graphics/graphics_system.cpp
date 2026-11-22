#include <omega/graphics/graphics_system.hpp>

#include <omega/platform/platform_system.hpp>

#include <ogfx/instance.hpp>
#include <ogfx/fence.hpp>
#include <ogfx/semaphore.hpp>
#include <ogfx/command_pool.hpp>

#include <array>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace omega
{

GraphicsSystem::GraphicsSystem(const std::string& application_name, uint32_t max_frames_in_flight,
                               PlatformSystem& platform, EventSystem& events, PerformanceSystem& performance)
    : m_instance(
      ogfx::InstanceDesc
      {
          .application_name = application_name.c_str(),
          .engine_name = "Omega Engine",
          .extensions = ogfx::InstanceExtension::DebugUtils | platform.runtime().required_instance_extensions()
      }),
      m_surface(platform.window().create_surface(m_instance)), 
      m_physical_device(m_instance.pick_physical_device(m_instance.enumerate_physical_devices())),
      m_device(m_physical_device, ogfx::DeviceDesc{}, &m_surface), 
      m_swapchain(m_physical_device, m_device, m_surface,
      ogfx::SwapchainDesc
      {
          .width = platform.window().width(),
          .height = platform.window().height(),
          .format = ogfx::Format::BGRA8_UNORM,
          .present_mode = ogfx::PresentMode::Immediate,
          .image_count = 3
      }),
      m_resource_allocator(m_device),
      m_max_frames_in_flight(std::max(max_frames_in_flight, 1u)),
      m_renderer(m_physical_device, m_device, m_resource_allocator),
      m_meshes(events), m_gpu_meshes(m_physical_device, m_device, m_renderer.resource_registry(), m_meshes, events), 
      m_textures(events), m_gpu_textures(m_physical_device, m_device, m_renderer.resource_registry(), m_textures, events),
      m_performance(performance)
{
    // Frame sync
    m_frames.resize(m_max_frames_in_flight);
    for (Frame& frame : m_frames)
    {
        ogfx::CommandPoolDesc pool_desc;
        pool_desc.transient = false;
        pool_desc.resettable = true;

        frame.command_pool = std::make_unique<ogfx::CommandPool>(m_device, m_device.queue(ogfx::QueueType::Graphics), pool_desc);
        frame.command_buffer = std::make_unique<ogfx::CommandBuffer>(*frame.command_pool);
        frame.upload_command_buffer = std::make_unique<ogfx::CommandBuffer>(*frame.command_pool);
        frame.image_available = std::make_unique<ogfx::Semaphore>(m_device);
        frame.in_flight = std::make_unique<ogfx::Fence>(m_device, true);
    }

    m_render_finished.reserve(m_swapchain.image_count());
    for (uint32_t i = 0; i < m_swapchain.image_count(); ++i)
    {
        m_render_finished.push_back(std::make_unique<ogfx::Semaphore>(m_device));
    }
}

GraphicsSystem::~GraphicsSystem() = default;

void GraphicsSystem::start_frame()
{
    ScopedCpuTimer timer(m_performance, "Graphics Start Frame");

    {
        ScopedCpuTimer timer(m_performance, "Wait Frame");
        wait_frame();
    }

    {
        ScopedCpuTimer timer(m_performance, "Complete Uploads");
        complete_uploads();
    }

    Frame& frame = m_frames[m_current_frame];

    {
        ScopedCpuTimer timer(m_performance, "Acquire Swapchain Image");

        m_frame_acquired = m_swapchain.acquire_next_image(*frame.image_available);
    }

    if (m_frame_acquired.status == ogfx::PresentResult::OutOfDate)
    {
        m_recreate_swapchain = true;
        return;
    }

    m_current_image_index = m_frame_acquired.image_index;

    if (m_frame_acquired.status == ogfx::PresentResult::Suboptimal)
        m_recreate_swapchain = true;
}

void GraphicsSystem::end_frame() 
{
    m_current_frame = (m_current_frame + 1) % m_max_frames_in_flight;
}

void GraphicsSystem::initialize()
{
    // Create default graphics assets
    const TextureAsset asset = create_missing_texture_asset();

    if (!m_textures.create_texture(asset, m_missing_texture))
        throw std::runtime_error("Failed to create missing texture.");

    if (!m_gpu_textures.make_resident(m_missing_texture))
        throw std::runtime_error("Failed to request missing texture residency.");

    m_missing_texture_view = m_gpu_textures.create_view(m_missing_texture, TextureViewDesc{});

    if (!m_missing_texture_view.is_valid())
        throw std::runtime_error("Failed to create missing texture view.");

    // Two synchronization passes are required:
    // 1. Upload the texture and wait for GPU completion
    // 2. Create its image views, which are deferred until the texture is resident
    Frame& frame = m_frames[m_current_frame];
    for (uint32_t i = 0; i < 2; ++i)
    {
        frame.upload_command_buffer->begin();
        const auto batch = m_gpu_textures.synchronize(*frame.upload_command_buffer);
        frame.upload_command_buffer->end();

        if (!batch.is_valid())
            continue;

        ogfx::SubmitDesc upload_submit;
        upload_submit.command_buffers.push_back(frame.upload_command_buffer.get());
        upload_submit.fence = frame.in_flight.get();

        frame.in_flight->reset();
        m_device.queue(ogfx::QueueType::Graphics).submit(upload_submit);

        // Wait for upload completion
        frame.in_flight->wait();

        m_gpu_textures.complete_uploads(batch);
        frame.upload_batch = {};
    }
}

void GraphicsSystem::update()
{
    ScopedCpuTimer timer(m_performance, "Graphics Update");

    Frame& frame = m_frames[m_current_frame];

    {
        ScopedCpuTimer timer(m_performance, "Synchronize Meshes");
        m_gpu_meshes.synchronize();
    }

    TextureUploadBatch batch;
    {
        ScopedCpuTimer timer(m_performance, "Synchronize Textures");

        frame.upload_command_buffer->begin();
        batch = m_gpu_textures.synchronize(*frame.upload_command_buffer);
        frame.upload_command_buffer->end();
    }

    if (batch.is_valid())
    {
        frame.upload_batch = batch;

        ScopedCpuTimer timer(m_performance, "Submit Uploads");

        ogfx::SubmitDesc upload_submit;
        upload_submit.command_buffers.push_back(frame.upload_command_buffer.get());

        // TODO: Change to transfer queue.
        m_device.queue(ogfx::QueueType::Graphics).submit(upload_submit);
    }
}

void GraphicsSystem::render()
{
    ScopedCpuTimer timer(m_performance, "Graphics Render");

    if (m_frame_acquired.status == ogfx::PresentResult::OutOfDate)
        return;

    Frame& frame = m_frames[m_current_frame];

    {
        ScopedCpuTimer timer(m_performance, "Renderer Execute");

        m_renderer.execute(*frame.command_buffer);
    }

    {
        ScopedCpuTimer timer(m_performance, "Graphics Submission");

        frame.in_flight->reset();

        ogfx::SubmitDesc submitDesc;
        submitDesc.command_buffers.push_back(frame.command_buffer.get());
        submitDesc.wait_semaphores.push_back({frame.image_available.get(), ogfx::PipelineStage::ColorAttachmentOutput});
        submitDesc.signal_semaphores.push_back(m_render_finished[m_current_image_index].get());
        submitDesc.fence = frame.in_flight.get();

        m_device.queue(ogfx::QueueType::Graphics).submit(submitDesc);
    }

    {
        ScopedCpuTimer timer(m_performance, "Graphics Present");

        ogfx::PresentResult present_result = m_device.queue(ogfx::QueueType::Present).present(m_swapchain, m_current_image_index, *m_render_finished[m_current_image_index]);

        if (present_result == ogfx::PresentResult::OutOfDate ||
            present_result == ogfx::PresentResult::Suboptimal)
        {
            m_recreate_swapchain = true;
        }
    }
}

void GraphicsSystem::wait()
{
    m_device.wait_idle();
}

void GraphicsSystem::wait_frame()
{
    Frame& frame = m_frames[m_current_frame];
    frame.in_flight->wait();
}

void GraphicsSystem::complete_uploads()
{
    Frame& frame = m_frames[m_current_frame];
    if (frame.upload_batch.is_valid())
    {
        m_gpu_textures.complete_uploads(frame.upload_batch);
        frame.upload_batch = {};
    }
}

uint32_t GraphicsSystem::current_frame() const 
{
    return m_current_frame;
}

uint32_t GraphicsSystem::current_image_index() const
{
    return m_current_image_index;
}

uint32_t GraphicsSystem::max_frames_in_flight() const
{
    return m_max_frames_in_flight;
}

bool GraphicsSystem::needs_swapchain_recreation() const
{
    return m_recreate_swapchain;
}

void GraphicsSystem::recreate_swapchain(uint32_t width, uint32_t height)
{
    m_device.wait_idle();

    m_swapchain.recreate(width, height);
    m_renderer.on_swapchain_recreated();
    m_recreate_swapchain = false;
}

ogfx::Swapchain& GraphicsSystem::swapchain()
{
    return m_swapchain;
}

orenderer::Renderer& GraphicsSystem::renderer()
{
    return m_renderer;
}

MeshSystem& GraphicsSystem::meshes()
{
    return m_meshes;
}

GpuMeshSystem& GraphicsSystem::gpu_meshes()
{
    return m_gpu_meshes;
}

ogfx::Instance& GraphicsSystem::instance()
{
    return m_instance;
}

ogfx::PhysicalDevice& GraphicsSystem::physical_device()
{
    return m_physical_device;
}

ogfx::Device& GraphicsSystem::device()
{
    return m_device;
}

ogfx::ResourceAllocator& GraphicsSystem::resource_allocator()
{
    return m_resource_allocator;
}

TextureSystem& GraphicsSystem::textures()
{
    return m_textures;
}

GpuTextureSystem& GraphicsSystem::gpu_textures()
{
    return m_gpu_textures;
}

TextureHandle GraphicsSystem::missing_texture() const
{
    return m_missing_texture;
}

TextureViewHandle GraphicsSystem::missing_texture_view() const
{
    return m_missing_texture_view;
}

ogfx::CommandBuffer& GraphicsSystem::current_command_buffer()
{
    return *m_frames[m_current_frame].command_buffer;
}

TextureAsset GraphicsSystem::create_missing_texture_asset()
{
    constexpr uint32_t width = 256;
    constexpr uint32_t height = 256;
    constexpr uint32_t tile_size = 16;

    constexpr std::array<uint8_t, 4> purple = {255, 0, 255, 255};
    constexpr std::array<uint8_t, 4> black = {0, 0, 0, 255};

    TextureAsset asset;
    asset.type = ogfx::ImageType::Image2D;
    asset.width = width;
    asset.height = height;
    asset.depth = 1;
    asset.mip_levels = 1;
    asset.array_layers = 1;
    asset.format = ogfx::Format::RGBA8_UNORM;

    TextureSubresource subresource;
    subresource.mip_level = 0;
    subresource.array_layer = 0;
    subresource.data.resize(width * height * 4);

    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            const auto& color = ((x / tile_size + y / tile_size) % 2 == 0) ? purple : black;

            const size_t offset = (static_cast<size_t>(y) * width + x) * 4;
            std::memcpy(subresource.data.data() + offset, color.data(), color.size());
        }
    }

    asset.subresources.push_back(std::move(subresource));
    return asset;
}

} // namespace omega