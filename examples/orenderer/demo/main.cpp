#include <oplatform/runtime.hpp>
#include <oplatform/window.hpp>

#include <ogfx/instance.hpp>
#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/resource_allocator.hpp>
#include <ogfx/swapchain.hpp>
#include <ogfx/command_pool.hpp>
#include <ogfx/fence.hpp>
#include <ogfx/semaphore.hpp>

#include <orenderer/renderer.hpp>

#include "renderer/pipelines/raster_pipeline.hpp"

#include "scene/geometry_generation.hpp"
#include "scene/directional_light.hpp"

#include <vector>

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

struct Frame
{
    std::unique_ptr<ogfx::CommandPool> command_pool;
    std::unique_ptr<ogfx::CommandBuffer> command_buffer;

    std::unique_ptr<ogfx::Semaphore> image_available;
    std::unique_ptr<ogfx::Fence> in_flight;
};

int main()
{
    // Window setup
    oplatform::Runtime runtime;

    oplatform::WindowDesc windowDesc;
    windowDesc.width = 1280;
    windowDesc.height = 720;
    windowDesc.title = "Renderer demo";
    windowDesc.resizable = true;
    windowDesc.fullscreen = false;

    oplatform::Window window(windowDesc);

    const auto extensions = runtime.required_instance_extensions();

    // Graphics framework setup
    ogfx::InstanceDesc instance_desc;
    instance_desc.application_name = "ogfx Instance Test";
    instance_desc.engine_name = "ographics";
    instance_desc.debug = true;
    instance_desc.extensions = extensions;

    ogfx::Instance instance(instance_desc);

    ogfx::Surface surface = window.create_surface(instance);

    const std::vector<ogfx::PhysicalDevice> devices = instance.enumerate_physical_devices();
    ogfx::PhysicalDevice physical_device = instance.pick_physical_device(devices);

    physical_device.desc();

    ogfx::DeviceDesc device_desc;

    ogfx::Device device(physical_device, device_desc, &surface);

    ogfx::ResourceAllocator resource_allocator(device);

    // Swapchain setup
    ogfx::SwapchainDesc swapchain_desc;
    swapchain_desc.width = window.width();
    swapchain_desc.height = window.height();
    swapchain_desc.format = ogfx::Format::BGRA8_UNORM;
    swapchain_desc.present_mode = ogfx::PresentMode::Immediate;
    swapchain_desc.image_count = 3;

    ogfx::Swapchain swapchain(physical_device, device, surface, swapchain_desc);
    uint32_t swapchainImageIndex = 0;

    // Submission queues
    ogfx::Queue& graphicsQueue = device.queue(ogfx::QueueType::Graphics);
    ogfx::Queue& presentQueue = device.queue(ogfx::QueueType::Present);

    // Frame sync
    std::array<Frame, MAX_FRAMES_IN_FLIGHT> frames;
    uint32_t current_frame = 0;

    for (Frame& frame : frames)
    {
        ogfx::CommandPoolDesc pool_desc;
        pool_desc.transient = false;
        pool_desc.resettable = true;

        frame.command_pool = std::make_unique<ogfx::CommandPool>(device, graphicsQueue, pool_desc);

        frame.command_buffer = std::make_unique<ogfx::CommandBuffer>(*frame.command_pool);

        frame.image_available = std::make_unique<ogfx::Semaphore>(device);

        frame.in_flight = std::make_unique<ogfx::Fence>(device, true);
    }

    std::vector<std::unique_ptr<ogfx::Semaphore>> render_finished;
    render_finished.reserve(swapchain.image_count());

    for (uint32_t i = 0; i < swapchain.image_count(); ++i)
    {
        render_finished.push_back(std::make_unique<ogfx::Semaphore>(device));
    }

    // Scene initialization
    SceneGeometry geometry;
    generate_sphere(geometry, 64, 32, 128);

    DirectionalLight light{};

    // Renderer
    orenderer::Renderer renderer(physical_device, device, resource_allocator);
    {
        orendergraph::ResourcePoolRegistry& registry = renderer.resource_registry();

        // Vertex buffer
        orendergraph::ResourceHandle<ogfx::Buffer> vertex_buffer_handle;
        {
            ogfx::BufferDesc desc;
            desc.size = sizeof(Vertex) * geometry.vertices.size();
            desc.usage = ogfx::BufferUsage::Vertex;
            desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

            vertex_buffer_handle = registry.create_resource<ogfx::Buffer>(physical_device, device, desc);
            ogfx::Buffer* buffer = registry.get_resource(vertex_buffer_handle);

            ogfx::BufferWriteInfo write_info;
            write_info.data = geometry.vertices.data();
            write_info.size = desc.size;
            buffer->write(write_info);
        }

        // Index buffer
        orendergraph::ResourceHandle<ogfx::Buffer> index_buffer_handle;
        {
            ogfx::BufferDesc desc;
            desc.size = sizeof(Index) * geometry.indices.size();
            desc.usage = ogfx::BufferUsage::Index;
            desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

            index_buffer_handle = registry.create_resource<ogfx::Buffer>(physical_device, device, desc);
            ogfx::Buffer* buffer = registry.get_resource(index_buffer_handle);

            ogfx::BufferWriteInfo write_info;
            write_info.data = geometry.indices.data();
            write_info.size = desc.size;
            buffer->write(write_info);
        }

        // Pipelines
        {
            renderer.create_pipeline<RasterPipeline>();

            renderer.set_active_pipeline(0);

            RasterPipelineDesc pipeline_desc{};

            pipeline_desc.deferred_feature_desc.vertex_buffer = vertex_buffer_handle;
            pipeline_desc.deferred_feature_desc.index_buffer = index_buffer_handle;
            pipeline_desc.deferred_feature_desc.swapchain_width = swapchain_desc.width;
            pipeline_desc.deferred_feature_desc.swapchain_height = swapchain_desc.height;
            pipeline_desc.deferred_feature_desc.clusters = &geometry.clusters;

            pipeline_desc.to_swapchain_feature_desc.swapchain_width = swapchain_desc.width;
            pipeline_desc.to_swapchain_feature_desc.swapchain_height = swapchain_desc.height;
            pipeline_desc.to_swapchain_feature_desc.image_index = &swapchainImageIndex;
            pipeline_desc.to_swapchain_feature_desc.swapchain_images.resize(swapchain.image_count());
            for (uint32_t i = 0; i < swapchain.image_count(); i++)
            {
                pipeline_desc.to_swapchain_feature_desc.swapchain_images[i] = registry.import_resource(&swapchain.image(i));
            }

            renderer.setup<RasterPipeline>(pipeline_desc);
            renderer.compile();
        }
    }

    while (!window.should_close())
    {
        window.poll_events();

        Frame& frame = frames[current_frame];

        frame.in_flight->wait();

        swapchainImageIndex = swapchain.acquire_next_image(*frame.image_available.get());

        ogfx::CommandBuffer& cmd = *frame.command_buffer;

        renderer.execute(cmd);

        frame.in_flight->reset();

        {
            ogfx::SubmitDesc submitDesc;
            submitDesc.command_buffers.push_back(&cmd);
            submitDesc.wait_semaphores.push_back({frame.image_available.get(), ogfx::PipelineStage::ColorAttachmentOutput});
            submitDesc.signal_semaphores.push_back(render_finished[swapchainImageIndex].get());
            submitDesc.fence = frame.in_flight.get();

            graphicsQueue.submit(submitDesc);

            ogfx::PresentResult presentResult = presentQueue.present(swapchain, swapchainImageIndex, *render_finished[swapchainImageIndex]);

            current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
        }
    }

    device.wait_idle();

    return 0;
}