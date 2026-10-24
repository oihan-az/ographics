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

#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <cstddef>
#include <array>
#include <cmath>
#include <chrono>
#include <numbers>

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




    ogfx::ImageDesc image_desc;
    image_desc.width = texture_width;
    image_desc.height = texture_height;
    image_desc.mip_levels = 1;
    image_desc.array_layers = 1;
    image_desc.format = ogfx::Format::RGBA8_UNORM;
    image_desc.usage = ogfx::ImageUsage::Storage | ogfx::ImageUsage::Sampled;

    ogfx::Image texture(physicalDevice, device, image_desc);
    texture.allocate_memory();
    texture.bind_memory();

    ogfx::ImageViewDesc image_view_desc;
    image_view_desc.image = &texture;
    image_view_desc.base_mip_level = 0;
    image_view_desc.mip_level_count = 1;
    image_view_desc.base_array_layer = 0;
    image_view_desc.array_layer_count = 1;

    ogfx::ImageView image_view(image_view_desc);




    ogfx::ResourceAllocator resource_allocator(device);

    ogfx::DescriptorPoolDesc pool_desc;
    pool_desc.max_sets = 2;
    pool_desc.sizes = {{ogfx::ResourceType::StorageTexture, 1},
                       {ogfx::ResourceType::SampledTexture, 1},
                       {ogfx::ResourceType::Sampler, 1}};

    resource_allocator.create_pool(pool_desc);




    ogfx::ResourceBindingDesc storage_texture_binding;
    storage_texture_binding.binding = 0;
    storage_texture_binding.type = ogfx::ResourceType::StorageTexture;
    storage_texture_binding.array_count = 1;
    storage_texture_binding.stages = ogfx::ShaderStageFlags::Compute;

    ogfx::ResourceSetLayoutDesc compute_set_layout_desc;
    compute_set_layout_desc.bindings = {storage_texture_binding};

    ogfx::ResourceSetLayout compute_set_layout(device, compute_set_layout_desc);

    ogfx::PipelineLayoutDesc compute_pipeline_layout_desc;
    compute_pipeline_layout_desc.set_layouts = {&compute_set_layout};

    ogfx::InlineConstantRange compute_constants_range;
    compute_constants_range.stages = ogfx::ShaderStageFlags::Compute;
    compute_constants_range.offset = 0;
    compute_constants_range.size = sizeof(ComputeConstants);

    compute_pipeline_layout_desc.inline_constant_ranges = {compute_constants_range};

    ogfx::PipelineLayout compute_pipeline_layout(device, compute_pipeline_layout_desc);

    ogfx::ResourceSetDesc compute_set_desc;
    compute_set_desc.layout = &compute_set_layout;

    ogfx::ResourceSet compute_set(device, compute_set_desc);
    compute_set.allocate(resource_allocator);

    ogfx::ImageBinding compute_image_binding;
    compute_image_binding.binding = 0;
    compute_image_binding.image_view = &image_view;
    compute_image_binding.image_state = ogfx::ImageState::Common;

    ogfx::ResourceSetWriteInfo compute_write_info;
    compute_write_info.textures = {compute_image_binding};

    compute_set.write(compute_write_info);




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

    ogfx::ResourceSetLayoutDesc graphics_set_layout_desc;
    graphics_set_layout_desc.bindings = {sampled_texture_binding, sampler_binding};

    ogfx::ResourceSetLayout graphics_set_layout(device, graphics_set_layout_desc);

    ogfx::PipelineLayoutDesc graphics_pipeline_layout_desc;
    graphics_pipeline_layout_desc.set_layouts = {&graphics_set_layout};

    ogfx::PipelineLayout graphics_pipeline_layout(device, graphics_pipeline_layout_desc);

    ogfx::ResourceSetDesc graphics_set_desc;
    graphics_set_desc.layout = &graphics_set_layout;

    ogfx::ResourceSet graphics_set(device, graphics_set_desc);
    graphics_set.allocate(resource_allocator);

    ogfx::ImageBinding sampled_image_binding;
    sampled_image_binding.binding = 0;
    sampled_image_binding.image_view = &image_view;
    sampled_image_binding.image_state = ogfx::ImageState::ShaderRead;

    ogfx::SamplerDesc sampler_desc;
    sampler_desc.min_filter = ogfx::Filter::Linear;
    sampler_desc.mag_filter = ogfx::Filter::Linear;
    sampler_desc.mip_filter = ogfx::MipmapFilter::Nearest;
    sampler_desc.address_u = ogfx::AddressMode::ClampToEdge;
    sampler_desc.address_v = ogfx::AddressMode::ClampToEdge;
    sampler_desc.address_w = ogfx::AddressMode::ClampToEdge;
    sampler_desc.max_lod = 0.0f;

    ogfx::Sampler sampler(device, sampler_desc);

    ogfx::SamplerBinding graphics_sampler_binding;
    graphics_sampler_binding.binding = 1;
    graphics_sampler_binding.sampler = &sampler;

    ogfx::ResourceSetWriteInfo graphics_write_info;
    graphics_write_info.textures = {sampled_image_binding};
    graphics_write_info.samplers = {graphics_sampler_binding};

    graphics_set.write(graphics_write_info);




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

    ogfx::Shader compute_shader(device, compute_shader_desc);
    ogfx::Shader vertex_shader(device, vertex_shader_desc);
    ogfx::Shader fragment_shader(device, fragment_shader_desc);



    ogfx::SwapchainDesc swapchainDesc;
    swapchainDesc.width = window.width();
    swapchainDesc.height = window.height();
    swapchainDesc.format = ogfx::Format::BGRA8_UNORM;
    swapchainDesc.present_mode = ogfx::PresentMode::FIFO;
    swapchainDesc.image_count = 3;

    ogfx::Swapchain swapchain(physicalDevice, device, surface, swapchainDesc);



    // Render pass
    ogfx::RenderPassAttachmentDesc color_attachment;
    color_attachment.format = swapchainDesc.format;
    color_attachment.load_op = ogfx::LoadOp::Clear;
    color_attachment.store_op = ogfx::StoreOp::Store;
    color_attachment.initial_state = ogfx::ImageState::Undefined;
    color_attachment.final_state = ogfx::ImageState::Present;

    ogfx::RenderPassDesc renderPassDesc;
    renderPassDesc.attachments.push_back(color_attachment);

    ogfx::RenderPass renderPass(device, renderPassDesc);




    // Framebuffers
    std::vector<ogfx::Framebuffer> framebuffers;
    framebuffers.reserve(swapchain.image_count());

    for (uint32_t i = 0; i < swapchain.image_count(); ++i)
    {
        ogfx::FramebufferDesc framebuffer_desc;
        framebuffer_desc.render_pass = &renderPass;
        framebuffer_desc.attachments = {&swapchain.image_view(i)};
        framebuffer_desc.width = swapchainDesc.width;
        framebuffer_desc.height = swapchainDesc.height;
        framebuffer_desc.layers = 1;

        framebuffers.emplace_back(device, framebuffer_desc);
    }




    ogfx::ComputePipelineDesc compute_pipeline_desc;
    compute_pipeline_desc.layout = &compute_pipeline_layout;
    compute_pipeline_desc.shader = &compute_shader;

    ogfx::ComputePipeline compute_pipeline(device, compute_pipeline_desc);




    // Graphics pipeline
    ogfx::GraphicsPipelineDesc graphicsPipelineDesc;

    graphicsPipelineDesc.render_pass = &renderPass;
    graphicsPipelineDesc.subpass = 0;

    graphicsPipelineDesc.layout = &graphics_pipeline_layout;

    // Shaders
    graphicsPipelineDesc.vertex_shader = &vertex_shader;
    graphicsPipelineDesc.fragment_shader = &fragment_shader;

    // Vertex input
    {
        // Vertex bindings
        ogfx::VertexBindingDesc vertex_binding;
        vertex_binding.binding = 0;
        vertex_binding.stride = sizeof(Vertex);
        vertex_binding.input_rate = ogfx::VertexInputRate::Vertex;

        graphicsPipelineDesc.vertex_input.bindings = {vertex_binding};

        // Vertex attributes
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

        graphicsPipelineDesc.vertex_input.attributes = {position_attribute, uv_attribute};
    }

    // Input assembly
    graphicsPipelineDesc.input_assembly.topology = ogfx::PrimitiveTopology::TriangleList;
    graphicsPipelineDesc.input_assembly.primitive_restart_enable = false;

    // Viewport
    graphicsPipelineDesc.viewport.dynamic_viewport = false;
    graphicsPipelineDesc.viewport.dynamic_scissor = false;

    ogfx::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchainDesc.width);
    viewport.height = static_cast<float>(swapchainDesc.height);
    viewport.min_depth = 0.0f;
    viewport.max_depth = 1.0f;
    graphicsPipelineDesc.viewport.viewports = {viewport};

    ogfx::Scissor scissor;
    scissor.x = 0;
    scissor.y = 0;
    scissor.width = swapchainDesc.width;
    scissor.height = swapchainDesc.height;
    graphicsPipelineDesc.viewport.scissors = {scissor};

    // Rasterization
    graphicsPipelineDesc.rasterization.polygon_mode = ogfx::PolygonMode::Fill;
    graphicsPipelineDesc.rasterization.cull_mode = ogfx::CullMode::None;
    graphicsPipelineDesc.rasterization.front_face = ogfx::FrontFace::CounterClockwise;

    // Multisampling
    graphicsPipelineDesc.multisampling.rasterization_samples = ogfx::SampleCount::Count1;

    // Color blending
    ogfx::ColorBlendAttachmentDesc colorBlendAttachment;
    colorBlendAttachment.blend_enable = false;
    colorBlendAttachment.write_mask = ogfx::ColorComponent::All;

    graphicsPipelineDesc.color_blend.attachments = {colorBlendAttachment};

    ogfx::GraphicsPipeline graphicsPipeline(device, graphicsPipelineDesc);




    // Command pool
    ogfx::Queue& graphicsQueue = device.queue(ogfx::QueueType::Graphics);

    ogfx::CommandPoolDesc commandPoolDesc;
    commandPoolDesc.transient = false;
    commandPoolDesc.resettable = true;

    ogfx::CommandPool commandPool(device, graphicsQueue, commandPoolDesc);

    ogfx::CommandBuffer commandBuffer(commandPool);




    ogfx::Fence compute_fence(device);
    compute_fence.reset();

    ogfx::Semaphore imageAvailable(device);
    ogfx::Semaphore renderFinished(device);

    ogfx::Fence inFlight(device);
    inFlight.reset();

    ogfx::Queue& presentQueue = device.queue(ogfx::QueueType::Present);

    const auto start_time = std::chrono::steady_clock::now();

    while (!window.should_close())
    {
        window.poll_events();

        const auto now = std::chrono::steady_clock::now();
        const float elapsed_seconds = std::chrono::duration<float>(now - start_time).count();

        {
            commandBuffer.begin();

            ogfx::ImageTransitionInfo to_storage;
            to_storage.image = &texture;
            to_storage.old_state = ogfx::ImageState::Undefined;
            to_storage.new_state = ogfx::ImageState::Common;

            commandBuffer.transition_image(to_storage);

            commandBuffer.bind_compute_pipeline(compute_pipeline);

            ogfx::ResourceSetBindInfo compute_bind_info;
            compute_bind_info.layout = &compute_pipeline_layout;
            compute_bind_info.resource_set = &compute_set;
            compute_bind_info.set_index = 0;
            compute_bind_info.pipeline_type = ogfx::PipelineType::Compute;

            commandBuffer.bind_resource_set(compute_bind_info);

            ComputeConstants compute_constants;
            compute_constants.width = texture_width;
            compute_constants.height = texture_height;
            compute_constants.t = elapsed_seconds;

            ogfx::InlineConstantsInfo constants_info;
            constants_info.layout = &compute_pipeline_layout;
            constants_info.stages = ogfx::ShaderStageFlags::Compute;
            constants_info.offset = 0;
            constants_info.size = sizeof(ComputeConstants);
            constants_info.data = &compute_constants;

            commandBuffer.set_inline_constants(constants_info);

            constexpr uint32_t workgroup_size_x = 8;
            constexpr uint32_t workgroup_size_y = 8;

            ogfx::DispatchInfo dispatch_info;
            dispatch_info.group_count_x = (texture_width + workgroup_size_x - 1) / workgroup_size_x;
            dispatch_info.group_count_y = (texture_height + workgroup_size_y - 1) / workgroup_size_y;
            dispatch_info.group_count_z = 1;

            commandBuffer.dispatch(dispatch_info);

            ogfx::ImageTransitionInfo to_sampled;
            to_sampled.image = &texture;
            to_sampled.old_state = ogfx::ImageState::Common;
            to_sampled.new_state = ogfx::ImageState::ShaderRead;

            commandBuffer.transition_image(to_sampled);

            commandBuffer.end();

            ogfx::SubmitDesc compute_submit_desc;
            compute_submit_desc.command_buffers.push_back(&commandBuffer);
            compute_submit_desc.fence = &compute_fence;

            graphicsQueue.submit(compute_submit_desc);
            compute_fence.wait();
            compute_fence.reset();
        }

        {
            const uint32_t imageIndex = swapchain.acquire_next_image(imageAvailable);

            commandBuffer.reset();
            commandBuffer.begin();

            ogfx::RenderPassBeginInfo render_pass_begin_info;
            render_pass_begin_info.offset_x = 0;
            render_pass_begin_info.offset_y = 0;
            render_pass_begin_info.width = swapchainDesc.width;
            render_pass_begin_info.height = swapchainDesc.height;

            render_pass_begin_info.clear_color = {0.1f, 0.1f, 0.1f, 1.0f};

            commandBuffer.begin_render_pass(renderPass, framebuffers[imageIndex], render_pass_begin_info);

            commandBuffer.bind_graphics_pipeline(graphicsPipeline);

            ogfx::ResourceSetBindInfo bind_info;
            bind_info.layout = &graphics_pipeline_layout;
            bind_info.resource_set = &graphics_set;
            bind_info.set_index = 0;

            commandBuffer.bind_resource_set(bind_info);

            ogfx::VertexBufferBindInfo vertex_buffer_bind_info;
            vertex_buffer_bind_info.buffer = &vertex_buffer;
            vertex_buffer_bind_info.binding = 0;
            vertex_buffer_bind_info.offset = 0;
            commandBuffer.bind_vertex_buffer(vertex_buffer_bind_info);

            ogfx::DrawInfo draw_info;
            draw_info.vertex_count = static_cast<uint32_t>(vertices.size());
            commandBuffer.draw(draw_info);

            commandBuffer.end_render_pass();

            commandBuffer.end();

            ogfx::SubmitDesc submitDesc;
            submitDesc.command_buffers.push_back(&commandBuffer);
            submitDesc.wait_semaphores.push_back({&imageAvailable, ogfx::PipelineStage::ColorAttachmentOutput});
            submitDesc.signal_semaphores.push_back(&renderFinished);
            submitDesc.fence = &inFlight;

            graphicsQueue.submit(submitDesc);

            inFlight.wait();
            inFlight.reset();

            ogfx::PresentResult presentResult = presentQueue.present(swapchain, imageIndex, renderFinished);
        }
    }

    device.wait_idle();

    return 0;
}