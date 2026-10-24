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

struct Vertex
{
    float position[3]{};
    float normal[3]{};
};

using Index = uint32_t;

struct MeshCluster
{
    float color[4]{1.0f, 1.0f, 1.0f, 1.0f};

    uint32_t first_index = 0;
    uint32_t index_count = 0;
    uint32_t padding[2]{};
};

struct SceneGeometry
{
    std::vector<Vertex> vertices;
    std::vector<Index> indices;
    std::vector<MeshCluster> clusters;
};

constexpr float PI = 3.14159265358979323846f;

void generate_sphere(SceneGeometry& geometry, uint32_t slices, uint32_t stacks, uint32_t triangles_per_cluster)
{
    geometry.vertices.clear();
    geometry.indices.clear();
    geometry.clusters.clear();

    if (slices < 3 || stacks < 2 || triangles_per_cluster == 0)
        return;

    // Generate sphere vertices
    for (uint32_t stack = 0; stack <= stacks; ++stack)
    {
        const float v = static_cast<float>(stack) / stacks;
        const float phi = v * PI;

        for (uint32_t slice = 0; slice <= slices; ++slice)
        {
            const float u = static_cast<float>(slice) / slices;
            const float theta = u * 2.0f * PI;

            Vertex vertex{};

            vertex.position[0] = std::sin(phi) * std::cos(theta);
            vertex.position[1] = std::cos(phi);
            vertex.position[2] = std::sin(phi) * std::sin(theta);

            vertex.normal[0] = vertex.position[0];
            vertex.normal[1] = vertex.position[1];
            vertex.normal[2] = vertex.position[2];

            geometry.vertices.push_back(vertex);
        }
    }

    const uint32_t cells_per_patch = std::max(1u, static_cast<uint32_t>(std::sqrt(static_cast<float>(triangles_per_cluster) / 2.0f)));

    const uint32_t patch_slices = cells_per_patch;
    const uint32_t patch_stacks = cells_per_patch;

    // Generate indices patch by patch, keeping each patch contiguous
    for (uint32_t stack_start = 0; stack_start < stacks; stack_start += patch_stacks)
    {
        const uint32_t stack_end = std::min(stack_start + patch_stacks, stacks);

        for (uint32_t slice_start = 0; slice_start < slices; slice_start += patch_slices)
        {
            const uint32_t slice_end = std::min(slice_start + patch_slices, slices);

            MeshCluster cluster{};
            cluster.first_index = static_cast<uint32_t>(geometry.indices.size());

            const uint32_t cluster_index = static_cast<uint32_t>(geometry.clusters.size());

            // Give each spatial patch a distinct color
            const float hue = static_cast<float>(cluster_index % 12) / 12.0f;

            cluster.color[0] = 0.5f + 0.5f * std::sin(hue * 6.28318f);
            cluster.color[1] = 0.5f + 0.5f * std::sin((hue + 0.3333f) * 6.28318f);
            cluster.color[2] = 0.5f + 0.5f * std::sin((hue + 0.6667f) * 6.28318f);
            cluster.color[3] = 1.0f;

            for (uint32_t stack = stack_start; stack < stack_end; ++stack)
            {
                for (uint32_t slice = slice_start; slice < slice_end; ++slice)
                {
                    const uint32_t row_size = slices + 1;

                    const uint32_t a = stack * row_size + slice;
                    const uint32_t b = a + row_size;
                    const uint32_t c = a + 1;
                    const uint32_t d = b + 1;

                    geometry.indices.push_back(a);
                    geometry.indices.push_back(b);
                    geometry.indices.push_back(c);

                    geometry.indices.push_back(c);
                    geometry.indices.push_back(b);
                    geometry.indices.push_back(d);
                }
            }

            cluster.index_count = static_cast<uint32_t>(geometry.indices.size()) - cluster.first_index;

            geometry.clusters.push_back(cluster);
        }
    }
}

struct DrawIndexedIndirectCommand
{
    uint32_t index_count = 0;
    uint32_t instance_count = 1;
    uint32_t first_index = 0;
    int32_t vertex_offset = 0;
    uint32_t first_instance = 0;
};

struct Mat4
{
    std::array<float, 16> elements{};

    Mat4 operator*(const Mat4& other) const
    {
        Mat4 result{};

        for (uint32_t col = 0; col < 4; ++col)
        {
            for (uint32_t row = 0; row < 4; ++row)
            {
                for (uint32_t k = 0; k < 4; ++k)
                {
                    result.elements[col * 4 + row] += elements[k * 4 + row] * other.elements[col * 4 + k];
                }
            }
        }

        return result;
    }
};

struct TransformData
{
    Mat4 model2w;
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
    deviceDesc.features = ogfx::DeviceFeature::MultiDrawIndirect | ogfx::DeviceFeature::DrawIndirectFirstInstance;
    ogfx::Device device(physicalDevice, deviceDesc, &surface);




    SceneGeometry geometry;
    generate_sphere(geometry, 1024, 512, 128);

    // Vertex buffer
    ogfx::BufferDesc vertex_buffer_desc;
    vertex_buffer_desc.size = sizeof(Vertex) * geometry.vertices.size();
    vertex_buffer_desc.usage = ogfx::BufferUsage::Vertex;
    vertex_buffer_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

    ogfx::Buffer vertex_buffer(physicalDevice, device, vertex_buffer_desc);

    vertex_buffer.allocate_memory();
    vertex_buffer.bind_memory();

    ogfx::BufferWriteInfo write_info;
    write_info.data = geometry.vertices.data();
    write_info.size = vertex_buffer_desc.size;

    vertex_buffer.write(write_info);

    // Index buffer
    ogfx::BufferDesc index_buffer_desc;
    index_buffer_desc.size = sizeof(Index) * geometry.indices.size();
    index_buffer_desc.usage = ogfx::BufferUsage::Index;
    index_buffer_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

    ogfx::Buffer index_buffer(physicalDevice, device, index_buffer_desc);

    index_buffer.allocate_memory();
    index_buffer.bind_memory();

    ogfx::BufferWriteInfo index_write_info;
    index_write_info.data = geometry.indices.data();
    index_write_info.size = index_buffer_desc.size;

    index_buffer.write(index_write_info);

    // Cluster metadata buffer
    ogfx::BufferDesc cluster_buffer_desc;
    cluster_buffer_desc.size = sizeof(MeshCluster) * geometry.clusters.size();
    cluster_buffer_desc.usage = ogfx::BufferUsage::Storage;
    cluster_buffer_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

    ogfx::Buffer cluster_buffer(physicalDevice, device, cluster_buffer_desc);

    cluster_buffer.allocate_memory();
    cluster_buffer.bind_memory();

    ogfx::BufferWriteInfo cluster_write_info;
    cluster_write_info.data = geometry.clusters.data();
    cluster_write_info.size = cluster_buffer_desc.size;

    cluster_buffer.write(cluster_write_info);

    // Indirect command buffer
    ogfx::BufferDesc indirect_buffer_desc;
    indirect_buffer_desc.size = sizeof(DrawIndexedIndirectCommand) * geometry.clusters.size();
    indirect_buffer_desc.usage = ogfx::BufferUsage::Storage | ogfx::BufferUsage::Indirect;
    indirect_buffer_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

    ogfx::Buffer indirect_buffer(physicalDevice, device, indirect_buffer_desc);

    indirect_buffer.allocate_memory();
    indirect_buffer.bind_memory();




    ogfx::BufferDesc uniform_buffer_desc;
    uniform_buffer_desc.size = sizeof(TransformData);
    uniform_buffer_desc.usage = ogfx::BufferUsage::Uniform;
    uniform_buffer_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

    ogfx::Buffer uniform_buffer(physicalDevice, device, uniform_buffer_desc);

    uniform_buffer.allocate_memory();
    uniform_buffer.bind_memory();




    ogfx::ResourceAllocator resource_allocator(device);

    ogfx::DescriptorPoolDesc pool_desc;
    pool_desc.max_sets = 2;
    pool_desc.sizes = {
        {ogfx::ResourceType::StorageBuffer, 3},
        {ogfx::ResourceType::UniformBuffer, 1}
    };

    resource_allocator.create_pool(pool_desc);




    ogfx::ResourceBindingDesc clusters_binding;
    clusters_binding.binding = 0;
    clusters_binding.type = ogfx::ResourceType::StorageBuffer;
    clusters_binding.array_count = 1;
    clusters_binding.stages = ogfx::ShaderStageFlags::Compute;

    ogfx::ResourceBindingDesc indirect_commands_binding;
    indirect_commands_binding.binding = 1;
    indirect_commands_binding.type = ogfx::ResourceType::StorageBuffer;
    indirect_commands_binding.array_count = 1;
    indirect_commands_binding.stages = ogfx::ShaderStageFlags::Compute;

    ogfx::ResourceSetLayoutDesc compute_set_layout_desc;
    compute_set_layout_desc.bindings = {clusters_binding, indirect_commands_binding};

    ogfx::ResourceSetLayout compute_set_layout(device, compute_set_layout_desc);

    ogfx::PipelineLayoutDesc compute_pipeline_layout_desc;
    compute_pipeline_layout_desc.set_layouts = {&compute_set_layout};

    ogfx::PipelineLayout compute_pipeline_layout(device, compute_pipeline_layout_desc);

    ogfx::ResourceSetDesc compute_set_desc;
    compute_set_desc.layout = &compute_set_layout;

    ogfx::ResourceSet compute_set(device, compute_set_desc);
    compute_set.allocate(resource_allocator);

    ogfx::ResourceSetWriteInfo compute_write_info;

    ogfx::BufferBinding clusters_buffer_binding;
    clusters_buffer_binding.binding = 0;
    clusters_buffer_binding.buffer = &cluster_buffer;
    clusters_buffer_binding.offset = 0;
    clusters_buffer_binding.size = cluster_buffer_desc.size;

    ogfx::BufferBinding indirect_buffer_binding;
    indirect_buffer_binding.binding = 1;
    indirect_buffer_binding.buffer = &indirect_buffer;
    indirect_buffer_binding.offset = 0;
    indirect_buffer_binding.size = indirect_buffer_desc.size;

    compute_write_info.buffers = {clusters_buffer_binding, indirect_buffer_binding};

    compute_set.write(compute_write_info);




    ogfx::ResourceBindingDesc cluster_binding;
    cluster_binding.binding = 0;
    cluster_binding.type = ogfx::ResourceType::StorageBuffer;
    cluster_binding.stages = ogfx::ShaderStageFlags::Vertex;

    ogfx::ResourceBindingDesc uniform_binding;
    uniform_binding.binding = 1;
    uniform_binding.type = ogfx::ResourceType::UniformBuffer;
    uniform_binding.array_count = 1;
    uniform_binding.stages = ogfx::ShaderStageFlags::Vertex;

    ogfx::ResourceSetLayoutDesc graphics_set_layout_desc;
    graphics_set_layout_desc.bindings = {
        cluster_binding,
        uniform_binding
    };

    ogfx::ResourceSetLayout graphics_set_layout(device, graphics_set_layout_desc);

    ogfx::PipelineLayoutDesc graphics_pipeline_layout_desc;
    graphics_pipeline_layout_desc.set_layouts = {&graphics_set_layout};

    ogfx::PipelineLayout graphics_pipeline_layout(device, graphics_pipeline_layout_desc);

    ogfx::ResourceSetDesc graphics_set_desc;
    graphics_set_desc.layout = &graphics_set_layout;

    ogfx::ResourceSet graphics_set(device, graphics_set_desc);
    graphics_set.allocate(resource_allocator);

    ogfx::ResourceSetWriteInfo graphics_write_info;

    ogfx::BufferBinding cluster_buffer_binding;
    cluster_buffer_binding.binding = 0;
    cluster_buffer_binding.buffer = &cluster_buffer;
    cluster_buffer_binding.offset = 0;
    cluster_buffer_binding.size = cluster_buffer_desc.size;

    ogfx::BufferBinding uniform_buffer_binding;
    uniform_buffer_binding.binding = 1;
    uniform_buffer_binding.buffer = &uniform_buffer;
    uniform_buffer_binding.offset = 0;
    uniform_buffer_binding.size = uniform_buffer_desc.size;

    graphics_write_info.buffers = {
        cluster_buffer_binding,
        uniform_buffer_binding
    };

    graphics_set.write(graphics_write_info);




    const std::filesystem::path shader_dir = "shaders/";
    const std::filesystem::path compute_shader_path = shader_dir / ("indirect_commands.comp." OGRAPHICS_SHADER_FORMAT);
    const std::filesystem::path vertex_shader_path = shader_dir / ("indirect_commands.vert." OGRAPHICS_SHADER_FORMAT);
    const std::filesystem::path fragment_shader_path = shader_dir / ("indirect_commands.frag." OGRAPHICS_SHADER_FORMAT);

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

        ogfx::VertexAttributeDesc normal_attribute;
        normal_attribute.location = 1;
        normal_attribute.binding = 0;
        normal_attribute.format = ogfx::Format::RGB32_FLOAT;
        normal_attribute.offset = offsetof(Vertex, normal);

        graphicsPipelineDesc.vertex_input.attributes = {position_attribute, normal_attribute};
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

        const auto now = std::chrono::steady_clock::now();
        const float elapsed_seconds = std::chrono::duration<float>(now - start_time).count();

        constexpr float rotation_speed = 1.0f;
        const float angle = elapsed_seconds * rotation_speed;

        const float c = std::cos(angle);
        const float s = std::sin(angle);

        Mat4 model2w{};
        model2w.elements[0] = c;
        model2w.elements[2] = s;
        model2w.elements[5] = 1.0f;
        model2w.elements[8] = -s;
        model2w.elements[10] = c;
        model2w.elements[15] = 1.0f;

        TransformData uniforms{};
        uniforms.model2w = model2w;

        ogfx::BufferWriteInfo uniform_write_info;
        uniform_write_info.data = &uniforms;
        uniform_write_info.size = sizeof(uniforms);
        uniform_write_info.offset = 0;

        uniform_buffer.write(uniform_write_info);

        commandBuffer.reset();
        commandBuffer.begin();

        {
            // Generate indexed indirect commands on the GPU
            commandBuffer.bind_compute_pipeline(compute_pipeline);

            ogfx::ResourceSetBindInfo compute_bind_info;
            compute_bind_info.layout = &compute_pipeline_layout;
            compute_bind_info.resource_set = &compute_set;
            compute_bind_info.set_index = 0;
            compute_bind_info.pipeline_type = ogfx::PipelineType::Compute;

            commandBuffer.bind_resource_set(compute_bind_info);

            ogfx::DispatchInfo dispatch_info;
            dispatch_info.group_count_x = static_cast<uint32_t>(geometry.clusters.size());
            dispatch_info.group_count_y = 1;
            dispatch_info.group_count_z = 1;

            commandBuffer.dispatch(dispatch_info);

            // Make compute shader writes visible to indirect drawing
            ogfx::BufferBarrierInfo indirect_barrier;
            indirect_barrier.buffer = &indirect_buffer;
            indirect_barrier.offset = 0;
            indirect_barrier.size = indirect_buffer_desc.size;
            indirect_barrier.memory_barrier_info.src_stages = ogfx::PipelineStage::ComputeShader;
            indirect_barrier.memory_barrier_info.dst_stages = ogfx::PipelineStage::DrawIndirect;
            indirect_barrier.memory_barrier_info.src_access = ogfx::AccessFlags::ShaderWrite;
            indirect_barrier.memory_barrier_info.dst_access = ogfx::AccessFlags::IndirectCommandRead;

            commandBuffer.buffer_barrier(indirect_barrier);
        }

        {
            const uint32_t imageIndex = swapchain.acquire_next_image(imageAvailable);

            ogfx::RenderPassBeginInfo render_pass_begin_info;
            render_pass_begin_info.offset_x = 0;
            render_pass_begin_info.offset_y = 0;
            render_pass_begin_info.width = swapchainDesc.width;
            render_pass_begin_info.height = swapchainDesc.height;
            render_pass_begin_info.clear_color = {0.1f, 0.1f, 0.1f, 1.0f};

            commandBuffer.begin_render_pass(renderPass, framebuffers[imageIndex], render_pass_begin_info);

            commandBuffer.bind_graphics_pipeline(graphicsPipeline);

            ogfx::ResourceSetBindInfo graphics_bind_info;
            graphics_bind_info.layout = &graphics_pipeline_layout;
            graphics_bind_info.resource_set = &graphics_set;
            graphics_bind_info.set_index = 0;
            graphics_bind_info.pipeline_type = ogfx::PipelineType::Graphics;

            commandBuffer.bind_resource_set(graphics_bind_info);

            ogfx::VertexBufferBindInfo vertex_buffer_bind_info;
            vertex_buffer_bind_info.buffer = &vertex_buffer;
            vertex_buffer_bind_info.binding = 0;
            vertex_buffer_bind_info.offset = 0;
            commandBuffer.bind_vertex_buffer(vertex_buffer_bind_info);

            ogfx::IndexBufferBindInfo index_buffer_bind_info;
            index_buffer_bind_info.buffer = &index_buffer;
            index_buffer_bind_info.offset = 0;
            index_buffer_bind_info.index_type = ogfx::IndexType::UInt32;
            commandBuffer.bind_index_buffer(index_buffer_bind_info);

            ogfx::DrawIndexedIndirectInfo draw_info;
            draw_info.buffer = &indirect_buffer;
            draw_info.offset = 0;
            draw_info.draw_count = static_cast<uint32_t>(geometry.clusters.size());
            draw_info.stride = sizeof(DrawIndexedIndirectCommand);
            commandBuffer.draw_indexed_indirect(draw_info);

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