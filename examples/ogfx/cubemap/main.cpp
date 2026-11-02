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

#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <cstddef>
#include <cmath>

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

constexpr uint32_t cubemap_size = 64;
constexpr uint32_t cubemap_face_count = 6;
constexpr uint32_t bytes_per_pixel = 4;

const std::array<std::array<uint8_t, 4>, cubemap_face_count> cubemap_colors = 
{{
    {{255, 0, 0, 255}},   // +X: Red
    {{0, 255, 0, 255}},   // -X: Green
    {{0, 0, 255, 255}},   // +Y: Blue
    {{255, 255, 0, 255}}, // -Y: Yellow
    {{255, 0, 255, 255}}, // +Z: Magenta
    {{0, 255, 255, 255}}  // -Z: Cyan
}};

// 5x7 bitmap font. Each row uses the lower 5 bits.
constexpr std::array<std::array<uint8_t, 7>, 6> letter_bitmaps = {
    {// R
     {{0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001}},
     // L
     {{0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111}},
     // T
     {{0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}},
     // D
     {{0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110}},
     // F
     {{0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000}},
     // B
     {{0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110}}}};

std::vector<uint8_t> generate_cubemap_pixels()
{
    const size_t face_size = static_cast<size_t>(cubemap_size) * cubemap_size * bytes_per_pixel;
    std::vector<uint8_t> pixels(face_size * cubemap_face_count);

    constexpr uint32_t glyph_width = 5;
    constexpr uint32_t glyph_height = 7;
    constexpr uint32_t scale = 5;

    const uint32_t start_x = (cubemap_size - glyph_width * scale) / 2;
    const uint32_t start_y = (cubemap_size - glyph_height * scale) / 2;

    for (uint32_t face = 0; face < cubemap_face_count; ++face)
    {
        const size_t face_offset = static_cast<size_t>(face) * face_size;

        // Fill the face with its existing background color
        for (size_t offset = 0; offset < face_size; offset += bytes_per_pixel)
        {
            for (uint32_t channel = 0; channel < bytes_per_pixel; ++channel)
            {
                pixels[face_offset + offset + channel] = cubemap_colors[face][channel];
            }
        }

        // Draw the centered letter in white
        const auto& bitmap = letter_bitmaps[face];
        for (uint32_t y = 0; y < glyph_height; ++y)
        {
            for (uint32_t x = 0; x < glyph_width; ++x)
            {
                if ((bitmap[y] & (1u << (glyph_width - 1 - x))) == 0)
                    continue;

                for (uint32_t dy = 0; dy < scale; ++dy)
                {
                    for (uint32_t dx = 0; dx < scale; ++dx)
                    {
                        const uint32_t pixel_x = start_x + x * scale + dx;
                        const uint32_t pixel_y = start_y + y * scale + dy;

                        const uint32_t pixel_offset = face_offset + (pixel_y * cubemap_size + pixel_x) * bytes_per_pixel;

                        pixels[pixel_offset + 0] = 255;
                        pixels[pixel_offset + 1] = 255;
                        pixels[pixel_offset + 2] = 255;
                        pixels[pixel_offset + 3] = 255;
                    }
                }
            }
        }
    }

    return pixels;
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




    ogfx::ImageDesc cubemap_desc;
    cubemap_desc.type = ogfx::ImageType::Cube;
    cubemap_desc.width = cubemap_size;
    cubemap_desc.height = cubemap_size;
    cubemap_desc.mip_levels = 1;
    cubemap_desc.array_layers = cubemap_face_count;
    cubemap_desc.format = ogfx::Format::RGBA8_UNORM;
    cubemap_desc.usage = ogfx::ImageUsage::TransferDst | ogfx::ImageUsage::Sampled;

    ogfx::Image cubemap(physicalDevice, device, cubemap_desc);
    cubemap.allocate_memory();
    cubemap.bind_memory();

    ogfx::ImageViewDesc cubemap_view_desc;
    cubemap_view_desc.image = &cubemap;
    cubemap_view_desc.type = ogfx::ImageViewType::Cube;
    cubemap_view_desc.base_mip_level = 0;
    cubemap_view_desc.mip_level_count = 1;
    cubemap_view_desc.base_array_layer = 0;
    cubemap_view_desc.array_layer_count = cubemap_face_count;

    ogfx::ImageView cubemap_view(cubemap_view_desc);

    const std::vector<uint8_t> cubemap_pixels = generate_cubemap_pixels();

    ogfx::BufferDesc staging_desc;
    staging_desc.size = cubemap_pixels.size();
    staging_desc.usage = ogfx::BufferUsage::TransferSrc;
    staging_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

    ogfx::Buffer staging_buffer(physicalDevice, device, staging_desc);
    staging_buffer.allocate_memory();
    staging_buffer.bind_memory();

    ogfx::BufferWriteInfo staging_write_info;
    staging_write_info.data = cubemap_pixels.data();
    staging_write_info.size = cubemap_pixels.size();
    staging_buffer.write(staging_write_info);

    ogfx::Queue& transfer_queue = device.queue(ogfx::QueueType::Graphics);

    ogfx::CommandPoolDesc upload_pool_desc;
    upload_pool_desc.transient = true;
    upload_pool_desc.resettable = false;

    ogfx::CommandPool upload_pool(device, transfer_queue, upload_pool_desc);

    ogfx::Fence upload_fence(device);
    upload_fence.reset();

    ogfx::CommandBuffer upload_command_buffer(upload_pool);
    upload_command_buffer.begin();

    ogfx::ImageTransitionInfo to_transfer_dst;
    to_transfer_dst.image = &cubemap;
    to_transfer_dst.old_state = ogfx::ImageState::Undefined;
    to_transfer_dst.new_state = ogfx::ImageState::TransferDst;
    to_transfer_dst.base_array_layer = 0;
    to_transfer_dst.array_layer_count = 6;
    upload_command_buffer.transition_image(to_transfer_dst);

    ogfx::BufferToImageCopyInfo copy_info;
    copy_info.buffer = &staging_buffer;
    copy_info.image = &cubemap;
    copy_info.width = cubemap_size;
    copy_info.height = cubemap_size;
    copy_info.base_array_layer = 0;
    copy_info.array_layer_count = cubemap_face_count;

    upload_command_buffer.copy_buffer_to_image(copy_info);

    ogfx::ImageTransitionInfo to_sampled;
    to_sampled.image = &cubemap;
    to_sampled.old_state = ogfx::ImageState::TransferDst;
    to_sampled.new_state = ogfx::ImageState::ShaderRead;
    to_sampled.base_array_layer = 0;
    to_sampled.array_layer_count = 6;
    upload_command_buffer.transition_image(to_sampled);

    upload_command_buffer.end();

    ogfx::SubmitDesc upload_submit_desc;
    upload_submit_desc.command_buffers.push_back(&upload_command_buffer);
    upload_submit_desc.fence = &upload_fence;

    transfer_queue.submit(upload_submit_desc);
    upload_fence.wait();




    ogfx::SamplerDesc sampler_desc;
    sampler_desc.min_filter = ogfx::Filter::Nearest;
    sampler_desc.mag_filter = ogfx::Filter::Nearest;
    sampler_desc.mip_filter = ogfx::MipmapFilter::Nearest;

    sampler_desc.address_u = ogfx::AddressMode::ClampToEdge;
    sampler_desc.address_v = ogfx::AddressMode::ClampToEdge;
    sampler_desc.address_w = ogfx::AddressMode::ClampToEdge;

    ogfx::Sampler sampler(device, sampler_desc);




    ogfx::ResourceAllocator resource_allocator(device);

    ogfx::DescriptorPoolDesc pool_desc;
    pool_desc.max_sets = 1;
    pool_desc.sizes = 
    {
    {ogfx::ResourceType::SampledTexture, 1},
    {ogfx::ResourceType::Sampler, 1}
    };
    resource_allocator.create_pool(pool_desc);




    ogfx::ResourceBindingDesc texture_binding;
    texture_binding.binding = 0;
    texture_binding.type = ogfx::ResourceType::SampledTexture;
    texture_binding.array_count = 1;
    texture_binding.stages = ogfx::ShaderStageFlags::Fragment;
    
    ogfx::ResourceBindingDesc sampler_binding;
    sampler_binding.binding = 1;
    sampler_binding.type = ogfx::ResourceType::Sampler;
    sampler_binding.array_count = 1;
    sampler_binding.stages = ogfx::ShaderStageFlags::Fragment;

    ogfx::ResourceSetLayoutDesc set_layout_desc;
    set_layout_desc.bindings = {texture_binding, sampler_binding};

    ogfx::ResourceSetLayout set_layout(device, set_layout_desc);




    ogfx::PipelineLayoutDesc pipeline_layout_desc;
    pipeline_layout_desc.set_layouts = {&set_layout};

    ogfx::PipelineLayout pipeline_layout(device, pipeline_layout_desc);




    ogfx::ResourceSetDesc set_desc;
    set_desc.layout = &set_layout;
    
    ogfx::ResourceSet texture_set(device, set_desc);
    texture_set.allocate(resource_allocator);

    ogfx::ImageBinding texture_set_binding;
    texture_set_binding.binding = 0;
    texture_set_binding.image_view = &cubemap_view;
    texture_set_binding.image_state = ogfx::ImageState::ShaderRead;

    ogfx::SamplerBinding sampler_set_binding;
    sampler_set_binding.binding = 1;
    sampler_set_binding.sampler = &sampler;

    ogfx::ResourceSetWriteInfo set_write_info;
    set_write_info.textures = {texture_set_binding};
    set_write_info.samplers = {sampler_set_binding};

    texture_set.write(set_write_info);




    const std::filesystem::path shader_dir = "shaders/";
    const std::filesystem::path vertex_shader_path = shader_dir / ("textured.vert." OGRAPHICS_SHADER_FORMAT);
    const std::filesystem::path fragment_shader_path = shader_dir / ("cubemap.frag." OGRAPHICS_SHADER_FORMAT);

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

    graphicsPipelineDesc.layout = &pipeline_layout;

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

    ogfx::Semaphore imageAvailable(device);
    ogfx::Semaphore renderFinished(device);

    ogfx::Fence inFlight(device);
    inFlight.reset();

    ogfx::Queue& presentQueue = device.queue(ogfx::QueueType::Present);

    const auto start_time = std::chrono::steady_clock::now();

    while (!window.should_close())
    {
        window.poll_events();

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
            render_pass_begin_info.render_pass = &renderPass;
            render_pass_begin_info.framebuffer = &framebuffers[imageIndex];
            commandBuffer.begin_render_pass(render_pass_begin_info);

            ogfx::PipelineBindInfo graphics_pipeline_bind_info;
            graphics_pipeline_bind_info.pipeline = &graphicsPipeline;
            commandBuffer.bind_pipeline(graphics_pipeline_bind_info);

            ogfx::ResourceSetBindInfo bind_info;
            bind_info.layout = &pipeline_layout;
            bind_info.resource_set = &texture_set;
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