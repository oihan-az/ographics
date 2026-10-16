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

#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <cstddef>

struct Vertex
{
    float position[3];
    float color[3];
};

const std::vector<Vertex> vertices =
{
    {{ 0.0f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }},
    {{ 0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }},
    {{-0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }}
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
    deviceDesc.enable_transfer = true;
    deviceDesc.enable_graphics = true;
    deviceDesc.enable_compute = true;
    deviceDesc.enable_present = true;

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




    const std::filesystem::path shader_dir = "shaders/";
    const std::filesystem::path vertex_shader_path = shader_dir / ("triangle_vertex_buffer.vert." OGRAPHICS_SHADER_FORMAT);
    const std::filesystem::path fragment_shader_path = shader_dir / ("triangle.frag." OGRAPHICS_SHADER_FORMAT);

    const std::vector<uint8_t> vertex_bytecode = load_shader(vertex_shader_path);
    const std::vector<uint8_t> fragment_bytecode = load_shader(fragment_shader_path);

    ogfx::ShaderDesc vertex_shader_desc;
    vertex_shader_desc.stage = ogfx::ShaderStage::Vertex;
    vertex_shader_desc.entry_point = "VSMain";
    vertex_shader_desc.bytecode = vertex_bytecode;

    ogfx::ShaderDesc fragment_shader_desc;
    fragment_shader_desc.stage = ogfx::ShaderStage::Fragment;
    fragment_shader_desc.entry_point = "PSMain";
    fragment_shader_desc.bytecode = fragment_bytecode;

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




    // Graphics pipeline
    ogfx::GraphicsPipelineDesc graphicsPipelineDesc;

    graphicsPipelineDesc.render_pass = &renderPass;
    graphicsPipelineDesc.subpass = 0;

    // Shaders
    graphicsPipelineDesc.shaders = { &vertex_shader, &fragment_shader };

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

        ogfx::VertexAttributeDesc color_attribute;
        color_attribute.location = 1;
        color_attribute.binding = 0;
        color_attribute.format = ogfx::Format::RGB32_FLOAT;
        color_attribute.offset = offsetof(Vertex, color);

        graphicsPipelineDesc.vertex_input.attributes = {position_attribute, color_attribute};
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

    ogfx::Semaphore imageAvailable(device);
    ogfx::Semaphore renderFinished(device);

    const uint32_t imageIndex = swapchain.acquire_next_image(imageAvailable);

    ogfx::Fence inFlight(device);

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

    ogfx::Queue& presentQueue = device.queue(ogfx::QueueType::Present);
    ogfx::PresentResult presentResult = presentQueue.present(swapchain, imageIndex, renderFinished);

    while (!window.should_close())
    {
        window.poll_events();
    }

    device.wait_idle();

    return 0;
}