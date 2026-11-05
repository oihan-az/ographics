#include <oplatform/runtime.hpp>
#include <oplatform/window.hpp>

#include <ogfx/instance.hpp>
#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/surface.hpp>
#include <ogfx/swapchain.hpp>
#include <ogfx/queue.hpp>
#include <ogfx/command_pool.hpp>
#include <ogfx/command_buffer.hpp>
#include <ogfx/semaphore.hpp>
#include <ogfx/fence.hpp>
#include <ogfx/render_pass.hpp>
#include <ogfx/framebuffer.hpp>
#include <ogfx/shader.hpp>
#include <ogfx/graphics_pipeline.hpp>
#include <ogfx/buffer.hpp>
#include <ogfx/resource_allocator.hpp>
#include <ogfx/resource_set_layout.hpp>
#include <ogfx/resource_set.hpp>
#include <ogfx/pipeline_layout.hpp>
#include <ogfx/sampler.hpp>
#include <ogfx/compute_pipeline.hpp>

#include <orendergraph/render_graph.hpp>

#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <chrono>

struct Vertex
{
    float position[3];
    float uv[2];
};

const std::vector<Vertex> vertices =
{
    {{ -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }},
    {{ -1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f }},
    {{  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f }},

    {{ -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }},
    {{  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f }},
    {{  1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }}
};

std::vector<uint8_t> load_shader(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file)
    {
        throw std::runtime_error("Failed to open shader: " + path.string());
    }

    const std::streamsize size = file.tellg();

    if (size <= 0)
    {
        throw std::runtime_error("Shader is empty: " + path.string());
    }

    std::vector<uint8_t> bytecode(static_cast<size_t>(size));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(bytecode.data()), size);

    if (!file)
    {
        throw std::runtime_error("Failed to read shader: " + path.string());
    }

    return bytecode;
}

const uint32_t texture_width = 512;
const uint32_t texture_height = 512;

struct ComputeConstants
{
    uint32_t width = 0;
    uint32_t height = 0;
    float t = 0;
};

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

struct Frame
{
    std::unique_ptr<ogfx::CommandPool> command_pool;
    std::unique_ptr<ogfx::CommandBuffer> command_buffer;

    std::unique_ptr<ogfx::Semaphore> image_available;
    std::unique_ptr<ogfx::Fence> in_flight;
};

uint32_t current_frame = 0;

int main()
{
    oplatform::Runtime runtime;

    oplatform::WindowDesc windowDesc;
    windowDesc.width = 1280;
    windowDesc.height = 720;
    windowDesc.title = "window example";
    windowDesc.resizable = true;
    windowDesc.fullscreen = false;

    oplatform::Window window(windowDesc);

    const auto extensions = runtime.required_instance_extensions();

    ogfx::InstanceDesc instanceDesc;
    instanceDesc.application_name = "ogfx Instance Test";
    instanceDesc.engine_name = "ographics";
    instanceDesc.debug = true;
    instanceDesc.extensions = extensions;

    ogfx::Instance instance(instanceDesc);

    ogfx::Surface surface = window.create_surface(instance);

    const std::vector<ogfx::PhysicalDevice> devices = instance.enumerate_physical_devices();
    ogfx::PhysicalDevice physicalDevice = instance.pick_physical_device(devices);

    physicalDevice.desc();

    ogfx::DeviceDesc deviceDesc;

    ogfx::Device device(physicalDevice, deviceDesc, &surface);



    std::array<Frame, MAX_FRAMES_IN_FLIGHT> frames;



    ogfx::SwapchainDesc swapchainDesc;
    swapchainDesc.width = window.width();
    swapchainDesc.height = window.height();
    swapchainDesc.format = ogfx::Format::BGRA8_UNORM;
    swapchainDesc.present_mode = ogfx::PresentMode::FIFO;
    swapchainDesc.image_count = 3;

    ogfx::Swapchain swapchain(physicalDevice, device, surface, swapchainDesc);




    ogfx::ResourceAllocator resource_allocator(device);




    orendergraph::RenderGraph graph(physicalDevice, device, resource_allocator);

    ogfx::ImageDesc image_desc;
    image_desc.width = texture_width;
    image_desc.height = texture_height;
    image_desc.mip_levels = 1;
    image_desc.array_layers = 1;
    image_desc.format = ogfx::Format::RGBA8_UNORM;
    image_desc.usage = ogfx::ImageUsage::Storage | ogfx::ImageUsage::Sampled;
    auto scratchTex = graph.create_resource<ogfx::Image>(image_desc);

    ogfx::ImageViewDesc image_view_desc;
    image_view_desc.image = graph.get_resource(scratchTex);
    image_view_desc.base_mip_level = 0;
    image_view_desc.mip_level_count = 1;
    image_view_desc.base_array_layer = 0;
    image_view_desc.array_layer_count = 1;
    auto scratchTexView = graph.create_resource<ogfx::ImageView>(image_view_desc);

    ogfx::SamplerDesc sampler_desc;
    sampler_desc.min_filter = ogfx::Filter::Linear;
    sampler_desc.mag_filter = ogfx::Filter::Linear;
    sampler_desc.mip_filter = ogfx::MipmapFilter::Nearest;
    sampler_desc.address_u = ogfx::AddressMode::ClampToEdge;
    sampler_desc.address_v = ogfx::AddressMode::ClampToEdge;
    sampler_desc.address_w = ogfx::AddressMode::ClampToEdge;
    sampler_desc.max_lod = 0.0f;
    auto sampler = graph.create_resource<ogfx::Sampler>(sampler_desc);




    ogfx::BufferDesc vertex_buffer_desc;
    vertex_buffer_desc.size = sizeof(Vertex) * vertices.size();
    vertex_buffer_desc.usage = ogfx::BufferUsage::Vertex;
    vertex_buffer_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

    ogfx::Buffer vertex_buffer(physicalDevice, device, vertex_buffer_desc);

    vertex_buffer.allocate_memory();
    vertex_buffer.bind_memory();

    ogfx::BufferWriteInfo write_info;
    write_info.data = vertices.data();
    write_info.size = vertex_buffer_desc.size;

    vertex_buffer.write(write_info);




    ogfx::DescriptorPoolDesc pool_desc;
    pool_desc.max_sets = 2;
    pool_desc.sizes = {{ogfx::ResourceType::StorageTexture, 1},
                       {ogfx::ResourceType::SampledTexture, 1},
                       {ogfx::ResourceType::Sampler, 1}};

    resource_allocator.create_pool(pool_desc);




    const std::filesystem::path shader_dir = "shaders/";
    const std::filesystem::path compute_shader_path = shader_dir / ("texture_compute.comp." OGRAPHICS_SHADER_FORMAT);
    const std::filesystem::path vertex_shader_path = shader_dir / ("textured.vert." OGRAPHICS_SHADER_FORMAT);
    const std::filesystem::path fragment_shader_path = shader_dir / ("textured.frag." OGRAPHICS_SHADER_FORMAT);

    const std::vector<uint8_t> compute_bytecode = load_shader(compute_shader_path);
    const std::vector<uint8_t> vertex_bytecode = load_shader(vertex_shader_path);
    const std::vector<uint8_t> fragment_bytecode = load_shader(fragment_shader_path);

    ogfx::ShaderDesc compute_shader_desc;
    compute_shader_desc.stage = ogfx::ShaderStage::Compute;
    compute_shader_desc.entry_point = "CSMain";
    compute_shader_desc.bytecode = compute_bytecode;

    ogfx::ShaderDesc vertex_shader_desc;
    vertex_shader_desc.stage = ogfx::ShaderStage::Vertex;
    vertex_shader_desc.entry_point = "VSMain";
    vertex_shader_desc.bytecode = vertex_bytecode;

    ogfx::ShaderDesc fragment_shader_desc;
    fragment_shader_desc.stage = ogfx::ShaderStage::Fragment;
    fragment_shader_desc.entry_point = "PSMain";
    fragment_shader_desc.bytecode = fragment_bytecode;




    ogfx::Queue& graphicsQueue = device.queue(ogfx::QueueType::Graphics);
    ogfx::Queue& presentQueue = device.queue(ogfx::QueueType::Present);

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




    const auto start_time = std::chrono::steady_clock::now();

    constexpr uint32_t workgroup_size_x = 8;
    constexpr uint32_t workgroup_size_y = 8;

    uint32_t imageIndex = 0;

    auto compute_node = graph.add_node<orendergraph::ComputeContext>
    (
        "Generate texture",
        [&](orendergraph::RenderGraph& rg, orendergraph::ComputeContext::Desc& desc)
        {
            rg.write(scratchTex, ogfx::ImageState::Common);

            desc.compute_shader_desc = compute_shader_desc;

            // Resource binding
            ogfx::ResourceBindingDesc storage_texture_binding;
            storage_texture_binding.binding = 0;
            storage_texture_binding.type = ogfx::ResourceType::StorageTexture;
            storage_texture_binding.array_count = 1;
            storage_texture_binding.stages = ogfx::ShaderStageFlags::Compute;

            ogfx::ResourceSetLayoutDesc set_layout_desc;
            set_layout_desc.bindings = {storage_texture_binding};

            desc.resource_set_layouts_desc.push_back(set_layout_desc);

            // Inline constants
            ogfx::InlineConstantRange constants_range;
            constants_range.stages = ogfx::ShaderStageFlags::Compute;
            constants_range.offset = 0;
            constants_range.size = sizeof(ComputeConstants);

            desc.inline_constants_ranges.push_back(constants_range);

            // Resource set
            orendergraph::ResourceSetDesc resource_set_desc;
            resource_set_desc.resource_set_layout_idx = 0;

            desc.resource_sets_desc.push_back(resource_set_desc);

            // Resource set write
            orendergraph::ResourceSetWriteInfo write_info;

            orendergraph::ImageBinding image_binding;
            image_binding.binding = 0;
            image_binding.image_view = scratchTexView;
            image_binding.image_state = ogfx::ImageState::Common;

            write_info.textures = {image_binding};

            orendergraph::ResourceWriteDesc resource_write_desc;
            resource_write_desc.resource_set_idx = 0;
            resource_write_desc.write_info = write_info;

            desc.resource_writes_desc.push_back(resource_write_desc);

            // Useful values
            desc.group_count_x = (texture_width + workgroup_size_x - 1) / workgroup_size_x;
            desc.group_count_y = (texture_height + workgroup_size_y - 1) / workgroup_size_y;
            desc.group_count_z = 1;
        }
    );

    compute_node->set_execute(
    [&](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd, orendergraph::RenderGraphNode<orendergraph::ComputeContext>& node)
    {
        orendergraph::ComputeContext& ctx = node.context();
        orendergraph::ComputeContext::Desc& desc = node.descriptor();

        const auto now = std::chrono::steady_clock::now();
        const float elapsed_seconds = std::chrono::duration<float>(now - start_time).count();

        ogfx::ResourceSetBindInfo compute_bind_info;
        compute_bind_info.layout = ctx.pipeline_layout;
        compute_bind_info.resource_set = ctx.resource_sets[0];
        compute_bind_info.set_index = 0;
        compute_bind_info.pipeline_type = ogfx::PipelineType::Compute;

        cmd.bind_resource_set(compute_bind_info);

        ComputeConstants compute_constants;
        compute_constants.width = texture_width;
        compute_constants.height = texture_height;
        compute_constants.t = elapsed_seconds;

        ogfx::InlineConstantsInfo constants_info;
        constants_info.layout = ctx.pipeline_layout;
        constants_info.stages = ogfx::ShaderStageFlags::Compute;
        constants_info.offset = 0;
        constants_info.size = sizeof(ComputeConstants);
        constants_info.data = &compute_constants;

        cmd.set_inline_constants(constants_info);

        cmd.dispatch({desc.group_count_x, desc.group_count_y, desc.group_count_z});
    });

    auto raster_node = graph.add_node<orendergraph::RasterContext>
    (
        "Textured",
        [&](orendergraph::RenderGraph& rg, orendergraph::RasterContext::Desc& desc)
        {
            rg.read(scratchTex, ogfx::ImageState::ShaderRead);

            // Render pass
            ogfx::RenderPassAttachmentDesc color_attachment;
            color_attachment.format = swapchainDesc.format;
            color_attachment.load_op = ogfx::LoadOp::Clear;
            color_attachment.store_op = ogfx::StoreOp::Store;
            color_attachment.initial_state = ogfx::ImageState::Undefined;
            color_attachment.final_state = ogfx::ImageState::Present;
            color_attachment.image_aspect = ogfx::ImageAspect::Color;

            desc.render_pass_desc.attachments.push_back(color_attachment);

            // Framebuffers
            desc.framebuffers_desc.reserve(swapchain.image_count());
            for (uint32_t i = 0; i < swapchain.image_count(); ++i)
            {
                ogfx::FramebufferDesc framebuffer_desc;
                framebuffer_desc.attachments = {&swapchain.image_view(i)};
                framebuffer_desc.width = swapchainDesc.width;
                framebuffer_desc.height = swapchainDesc.height;
                framebuffer_desc.layers = 1;

                desc.framebuffers_desc.push_back(framebuffer_desc);
            }

            // Shaders
            desc.vertex_shader_desc = vertex_shader_desc;
            desc.fragment_shader_desc = fragment_shader_desc;

            // Resource bindings
            ogfx::ResourceBindingDesc sampled_texture_binding;
            sampled_texture_binding.binding = 0;
            sampled_texture_binding.type = ogfx::ResourceType::SampledTexture;
            sampled_texture_binding.array_count = 1;
            sampled_texture_binding.stages = ogfx::ShaderStageFlags::Fragment;

            ogfx::ResourceBindingDesc sampler_binding;
            sampler_binding.binding = 1;
            sampler_binding.type = ogfx::ResourceType::Sampler;
            sampler_binding.array_count = 1;
            sampler_binding.stages = ogfx::ShaderStageFlags::Fragment;

            ogfx::ResourceSetLayoutDesc set_layout_desc;
            set_layout_desc.bindings = {sampled_texture_binding, sampler_binding};

            desc.resource_set_layouts_desc.push_back(set_layout_desc);

            // Resource set
            orendergraph::ResourceSetDesc resource_set_desc;
            resource_set_desc.resource_set_layout_idx = 0;

            desc.resource_sets_desc.push_back(resource_set_desc);

            // Resource set writes
            orendergraph::ResourceWriteDesc resource_write_desc;

            orendergraph::ImageBinding sampled_image_binding;
            sampled_image_binding.binding = 0;
            sampled_image_binding.image_view = scratchTexView;
            sampled_image_binding.image_state = ogfx::ImageState::ShaderRead;

            orendergraph::SamplerBinding sampler_write_binding;
            sampler_write_binding.binding = 1;
            sampler_write_binding.sampler = sampler;

            orendergraph::ResourceSetWriteInfo write_info;
            write_info.textures = {sampled_image_binding};
            write_info.samplers = {sampler_write_binding};

            resource_write_desc.resource_set_idx = 0;
            resource_write_desc.write_info = write_info;

            desc.resource_writes_desc.push_back(resource_write_desc);

            // Vertex input
            ogfx::VertexBindingDesc vertex_binding;
            vertex_binding.binding = 0;
            vertex_binding.stride = sizeof(Vertex);
            vertex_binding.input_rate = ogfx::VertexInputRate::Vertex;

            desc.vertex_input.bindings = {vertex_binding};

            ogfx::VertexAttributeDesc position_attribute;
            position_attribute.location = 0;
            position_attribute.binding = 0;
            position_attribute.format = ogfx::Format::RGB32_FLOAT;
            position_attribute.offset = offsetof(Vertex, position);

            ogfx::VertexAttributeDesc uv_attribute;
            uv_attribute.location = 1;
            uv_attribute.binding = 0;
            uv_attribute.format = ogfx::Format::RG32_FLOAT;
            uv_attribute.offset = offsetof(Vertex, uv);

            desc.vertex_input.attributes = {position_attribute, uv_attribute};

            // Input assembly
            desc.input_assembly.topology = ogfx::PrimitiveTopology::TriangleList;
            desc.input_assembly.primitive_restart_enable = false;

            // Viewport
            desc.viewport.dynamic_viewport = false;
            desc.viewport.dynamic_scissor = false;

            ogfx::Viewport viewport;
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapchainDesc.width);
            viewport.height = static_cast<float>(swapchainDesc.height);
            viewport.min_depth = 0.0f;
            viewport.max_depth = 1.0f;

            desc.viewport.viewports = {viewport};

            ogfx::Scissor scissor;
            scissor.x = 0;
            scissor.y = 0;
            scissor.width = swapchainDesc.width;
            scissor.height = swapchainDesc.height;

            desc.viewport.scissors = {scissor};

            // Rasterization
            desc.rasterization.polygon_mode = ogfx::PolygonMode::Fill;
            desc.rasterization.cull_mode = ogfx::CullMode::None;
            desc.rasterization.front_face = ogfx::FrontFace::CounterClockwise;

            // Multisampling
            desc.multisampling.rasterization_samples = ogfx::SampleCount::Count1;

            // Color blending
            ogfx::ColorBlendAttachmentDesc color_blend_attachment;
            color_blend_attachment.blend_enable = false;
            color_blend_attachment.write_mask = ogfx::ColorComponent::All;

            desc.color_blend.attachments = {color_blend_attachment};

            // Runtime render settings
            desc.framebuffer_index = 0;
            desc.width = swapchainDesc.width;
            desc.height = swapchainDesc.height;
            desc.clear_values = {{.type = ogfx::ClearValueType::Color, .color = {0.0f, 0.0f, 0.0f, 1.0f}}};
        }
    );

    raster_node->set_pre_execute(
    [&](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd, orendergraph::RenderGraphNode<orendergraph::RasterContext>& node)
    {
        orendergraph::RasterContext::Desc& desc = node.descriptor();
        desc.framebuffer_index = imageIndex;

        orendergraph::NodePreExecute<orendergraph::RasterContext>::execute(graph, cmd, *raster_node);
    });

    raster_node->set_execute(
    [&](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd, orendergraph::RenderGraphNode<orendergraph::RasterContext>& node)
    {
        orendergraph::RasterContext& ctx = node.context();

        ogfx::ResourceSetBindInfo bind_info;
        bind_info.layout = ctx.pipeline_layout;
        bind_info.resource_set = ctx.resource_sets[0];
        bind_info.set_index = 0;
        bind_info.pipeline_type = ogfx::PipelineType::Graphics;

        cmd.bind_resource_set(bind_info);

        ogfx::VertexBufferBindInfo vertex_buffer_bind_info;
        vertex_buffer_bind_info.buffer = &vertex_buffer;
        vertex_buffer_bind_info.binding = 0;
        vertex_buffer_bind_info.offset = 0;
        cmd.bind_vertex_buffer(vertex_buffer_bind_info);

        ogfx::DrawInfo draw_info;
        draw_info.vertex_count = static_cast<uint32_t>(vertices.size());
        cmd.draw(draw_info);
    });
    
    graph.compile();

    while (!window.should_close())
    {
        window.poll_events();

        Frame& frame = frames[current_frame];

        frame.in_flight->wait();

        imageIndex = swapchain.acquire_next_image(*frame.image_available.get());

        ogfx::CommandBuffer& cmd = *frame.command_buffer;
        graph.execute(cmd);

        frame.in_flight->reset();

        {
            ogfx::SubmitDesc submitDesc;
            submitDesc.command_buffers.push_back(&cmd);
            submitDesc.wait_semaphores.push_back({frame.image_available.get(), ogfx::PipelineStage::ColorAttachmentOutput});
            submitDesc.signal_semaphores.push_back(render_finished[imageIndex].get());
            submitDesc.fence = frame.in_flight.get();

            graphicsQueue.submit(submitDesc);

            ogfx::PresentResult presentResult = presentQueue.present(swapchain, imageIndex, *render_finished[imageIndex]);

            current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
        }
    }

    device.wait_idle();

    return 0;
}