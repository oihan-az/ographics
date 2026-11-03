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
    float position[4]{};
    float normal[4]{};
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

    const uint32_t cells_per_patch =
        std::max(1u, static_cast<uint32_t>(std::sqrt(static_cast<float>(triangles_per_cluster) / 2.0f)));

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
    deviceDesc.features = ogfx::DeviceFeature::RayTracing;
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

    std::vector<orendergraph::ResourceHandle<ogfx::Image>> swapchain_image_handles;
    swapchain_image_handles.resize(swapchainDesc.image_count);
    for (uint32_t i = 0; i < swapchainDesc.image_count; i++)
    {
        swapchain_image_handles[i] = graph.import_resource(&swapchain.image(i));
    }

    ogfx::ImageDesc output_image_desc{};
    output_image_desc.width = windowDesc.width;
    output_image_desc.height = windowDesc.height;
    output_image_desc.format = ogfx::Format::RGBA32_FLOAT;
    output_image_desc.usage = ogfx::ImageUsage::Storage | ogfx::ImageUsage::TransferSrc;

    auto rt_output_image = graph.create_resource<ogfx::Image>(output_image_desc);

    ogfx::ImageViewDesc image_view_desc;
    image_view_desc.image = graph.get_resource(rt_output_image);
    auto image_view = graph.create_resource<ogfx::ImageView>(image_view_desc);

    SceneGeometry geometry;
    generate_sphere(geometry, 64, 32, 32);

    // Vertex buffer
    ogfx::BufferDesc vertex_buffer_desc;
    vertex_buffer_desc.size = sizeof(Vertex) * geometry.vertices.size();
    vertex_buffer_desc.usage = ogfx::BufferUsage::Vertex | ogfx::BufferUsage::Storage |
                               ogfx::BufferUsage::AccelerationStructureBuildInput |
                               ogfx::BufferUsage::ShaderDeviceAddress;
    vertex_buffer_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

    ogfx::Buffer vertex_buffer(physicalDevice, device, vertex_buffer_desc);

    vertex_buffer.allocate_memory();
    vertex_buffer.bind_memory();

    ogfx::BufferWriteInfo write_info;
    write_info.data = geometry.vertices.data();
    write_info.size = vertex_buffer_desc.size;

    vertex_buffer.write(write_info);

    auto vertex_buffer_handle = graph.import_resource<ogfx::Buffer>(&vertex_buffer);

    // Index buffer
    ogfx::BufferDesc index_buffer_desc;
    index_buffer_desc.size = sizeof(Index) * geometry.indices.size();
    index_buffer_desc.usage = ogfx::BufferUsage::Index | ogfx::BufferUsage::Storage |
                              ogfx::BufferUsage::AccelerationStructureBuildInput |
                              ogfx::BufferUsage::ShaderDeviceAddress;
    index_buffer_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

    ogfx::Buffer index_buffer(physicalDevice, device, index_buffer_desc);

    index_buffer.allocate_memory();
    index_buffer.bind_memory();

    ogfx::BufferWriteInfo index_write_info;
    index_write_info.data = geometry.indices.data();
    index_write_info.size = index_buffer_desc.size;

    index_buffer.write(index_write_info);

    auto index_buffer_handle = graph.import_resource<ogfx::Buffer>(&index_buffer);

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

    auto cluster_buffer_handle = graph.import_resource<ogfx::Buffer>(&cluster_buffer);




    ogfx::DescriptorPoolDesc pool_desc;
    pool_desc.max_sets = 4;
    pool_desc.sizes = {{ogfx::ResourceType::StorageTexture, 1},
                       {ogfx::ResourceType::AccelerationStructure, 1},
                       {ogfx::ResourceType::StorageBuffer, 3}};

    resource_allocator.create_pool(pool_desc);




    const std::filesystem::path shader_dir = "shaders/";
    const std::filesystem::path raygen_shader_path = shader_dir / ("raygen.rgen." OGRAPHICS_SHADER_FORMAT);
    const std::filesystem::path miss_shader_path = shader_dir / ("miss.rmiss." OGRAPHICS_SHADER_FORMAT);
    const std::filesystem::path closesthit_shader_path = shader_dir / ("closesthit.rchit." OGRAPHICS_SHADER_FORMAT);

    const std::vector<uint8_t> raygen_bytecode = load_shader(raygen_shader_path);
    const std::vector<uint8_t> miss_bytecode = load_shader(miss_shader_path);
    const std::vector<uint8_t> closesthit_bytecode = load_shader(closesthit_shader_path);

    ogfx::ShaderDesc raygen_desc;
    raygen_desc.stage = ogfx::ShaderStage::RayGen;
    raygen_desc.entry_point = "RGMain";
    raygen_desc.bytecode = raygen_bytecode;

    ogfx::ShaderDesc miss_desc;
    miss_desc.stage = ogfx::ShaderStage::Miss;
    miss_desc.entry_point = "MSMain";
    miss_desc.bytecode = miss_bytecode;

    ogfx::ShaderDesc closesthit_desc;
    closesthit_desc.stage = ogfx::ShaderStage::ClosestHit;
    closesthit_desc.entry_point = "CHMain";
    closesthit_desc.bytecode = closesthit_bytecode;




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

    auto* blas_node = graph.add_node<orendergraph::BLASContext>
    (
        "Build BLAS",
        [&](orendergraph::RenderGraph& rg, orendergraph::BLASContext::Desc& desc)
        {
            for (const MeshCluster& cluster : geometry.clusters)
            {
                orendergraph::AccelerationStructureTriangleGeometry triangle{};

                triangle.vertex_buffer = vertex_buffer_handle;
                triangle.vertex_offset = 0;
                triangle.vertex_stride = sizeof(Vertex);
                triangle.vertex_count = static_cast<uint32_t>(geometry.vertices.size());
                triangle.vertex_format = ogfx::Format::RGB32_FLOAT;

                triangle.index_buffer = index_buffer_handle;
                triangle.index_offset = static_cast<uint64_t>(cluster.first_index) * sizeof(Index);
                triangle.index_count = cluster.index_count;
                triangle.index_type = ogfx::IndexType::UInt32;
                triangle.primitive_count = cluster.index_count / 3;

                orendergraph::BLASContext::Desc::GeometryDesc geometry_desc{};
                geometry_desc.triangle_geometries.push_back(triangle);

                desc.geometries_desc.push_back(std::move(geometry_desc));
            }
        }
    );

    blas_node->set_execution_frequency(orendergraph::ExecutionFrequency::Once);

    auto* tlas_node = graph.add_node<orendergraph::TLASContext>
    (
        "Build TLAS",
        [&](orendergraph::RenderGraph& rg, orendergraph::TLASContext::Desc& desc)
        {
            orendergraph::BLASContext ctx = blas_node->context();

            const auto& blases = ctx.blases;
            desc.instances.reserve(blases.size());
            for (uint32_t i = 0; i < blases.size(); ++i)
            {
                orendergraph::AccelerationStructureInstanceDesc instance{};

                instance.blas = ctx.blas_handles[i];

                instance.transform[0][0] = 1.0f;
                instance.transform[0][1] = 0.0f;
                instance.transform[0][2] = 0.0f;
                instance.transform[0][3] = 0.0f;

                instance.transform[1][0] = 0.0f;
                instance.transform[1][1] = 1.0f;
                instance.transform[1][2] = 0.0f;
                instance.transform[1][3] = 0.0f;

                instance.transform[2][0] = 0.0f;
                instance.transform[2][1] = 0.0f;
                instance.transform[2][2] = 1.0f;
                instance.transform[2][3] = 0.0f;

                instance.custom_index = i;
                instance.mask = 0xFF;
                instance.shader_binding_table_record_offset = 0;
                instance.flags = 0;

                desc.instances.push_back(instance);

                rg.read(ctx.blas_handles[i], {});
            }
        }
    );

    tlas_node->set_execution_frequency(orendergraph::ExecutionFrequency::Once);
    
    auto* ray_tracing_node = graph.add_node<orendergraph::RayTracingContext>
    (
        "Ray Tracing",
        [&](orendergraph::RenderGraph& rg, orendergraph::RayTracingContext::Desc& desc)
        {
            desc.raygen_shader_desc = raygen_desc;

            desc.miss_shader_descs.push_back(miss_desc);

            orendergraph::RayTracingHitShadersDesc hit_shaders{};
            hit_shaders.closest_hit_shader_desc = closesthit_desc;
            desc.hit_shader_descs.push_back(hit_shaders);

            desc.max_recursion_depth = 1;
            desc.width = swapchainDesc.width;
            desc.height = swapchainDesc.height;

            ogfx::ResourceBindingDesc tlas_binding{};
            tlas_binding.binding = 0;
            tlas_binding.type = ogfx::ResourceType::AccelerationStructure;
            tlas_binding.array_count = 1;
            tlas_binding.stages = ogfx::ShaderStageFlags::RayGen | ogfx::ShaderStageFlags::ClosestHit;

            ogfx::ResourceBindingDesc output_image_binding{};
            output_image_binding.binding = 1;
            output_image_binding.type = ogfx::ResourceType::StorageTexture;
            output_image_binding.array_count = 1;
            output_image_binding.stages = ogfx::ShaderStageFlags::RayGen;

            ogfx::ResourceBindingDesc vertices_binding{};
            vertices_binding.binding = 2;
            vertices_binding.type = ogfx::ResourceType::StorageBuffer;
            vertices_binding.array_count = 1;
            vertices_binding.stages = ogfx::ShaderStageFlags::ClosestHit;

            ogfx::ResourceBindingDesc indices_binding{};
            indices_binding.binding = 3;
            indices_binding.type = ogfx::ResourceType::StorageBuffer;
            indices_binding.array_count = 1;
            indices_binding.stages = ogfx::ShaderStageFlags::ClosestHit;

            ogfx::ResourceBindingDesc clusters_binding{};
            clusters_binding.binding = 4;
            clusters_binding.type = ogfx::ResourceType::StorageBuffer;
            clusters_binding.array_count = 1;
            clusters_binding.stages = ogfx::ShaderStageFlags::ClosestHit;

            ogfx::ResourceSetLayoutDesc rt_set_layout_desc{};
            rt_set_layout_desc.bindings = 
            {
                tlas_binding, 
                output_image_binding, 
                vertices_binding, 
                indices_binding,
                clusters_binding
            };

            desc.resource_set_layouts_desc.push_back(rt_set_layout_desc);

            // Resource set
            orendergraph::ResourceSetDesc resource_set_desc;
            resource_set_desc.resource_set_layout_idx = 0;

            desc.resource_sets_desc.push_back(resource_set_desc);

            // Resource set write
            orendergraph::ResourceWriteDesc resource_write_desc;
            resource_write_desc.resource_set_idx = 0;

            orendergraph::ImageBinding rt_output_image_write{};
            rt_output_image_write.binding = 1;
            rt_output_image_write.image_view = image_view;
            rt_output_image_write.image_state = ogfx::ImageState::Common;

            orendergraph::TLASBinding rt_tlas_write{};
            rt_tlas_write.binding = 0;
            rt_tlas_write.tlas = tlas_node->context().tlas_handle;

            orendergraph::BufferBinding rt_vertex_buffer_write{};
            rt_vertex_buffer_write.binding = 2;
            rt_vertex_buffer_write.buffer = vertex_buffer_handle;
            rt_vertex_buffer_write.range = vertex_buffer_desc.size;

            orendergraph::BufferBinding rt_index_buffer_write{};
            rt_index_buffer_write.binding = 3;
            rt_index_buffer_write.buffer = index_buffer_handle;
            rt_index_buffer_write.range = index_buffer_desc.size;

            orendergraph::BufferBinding rt_clusters_buffer_write{};
            rt_clusters_buffer_write.binding = 4;
            rt_clusters_buffer_write.buffer = cluster_buffer_handle;
            rt_clusters_buffer_write.range = cluster_buffer_desc.size;

            resource_write_desc.write_info.textures = {rt_output_image_write};
            resource_write_desc.write_info.tlases = {rt_tlas_write};
            resource_write_desc.write_info.buffers = {rt_vertex_buffer_write, rt_index_buffer_write, rt_clusters_buffer_write};

            desc.resource_writes_desc.push_back(resource_write_desc);

            rg.read(tlas_node->context().tlas_handle, {});
            rg.write(rt_output_image, {}, {});
        }
    );

    ray_tracing_node->set_execute
    (
        [](orendergraph::RenderGraph&, ogfx::CommandBuffer& cmd, orendergraph::RenderGraphNode<orendergraph::RayTracingContext>& node)
        {
            auto& ctx = node.context();
            auto& desc = node.descriptor();

            ogfx::ResourceSetBindInfo resource_set_bind_info{};
            resource_set_bind_info.set_index = 0;
            resource_set_bind_info.pipeline_type = ogfx::PipelineType::RayTracing;
            resource_set_bind_info.resource_set = ctx.resource_sets[0];
            resource_set_bind_info.layout = ctx.pipeline_layout;
            cmd.bind_resource_set(resource_set_bind_info);

            ogfx::TraceRaysInfo trace_rays_info;
            trace_rays_info.width = desc.width;
            trace_rays_info.height = desc.height;
            trace_rays_info.depth = 1;
            trace_rays_info.pipeline = ctx.pipeline;
            trace_rays_info.sbt_buffer = ctx.sbt_buffer;

            cmd.trace_rays(trace_rays_info);
        }
    );

    auto* blit_node = graph.add_node<orendergraph::BlitContext>
    (
        "Blit to Swapchain",
        [&](orendergraph::RenderGraph& rg, orendergraph::BlitContext::Desc& desc)
        {
            desc.source_image = rt_output_image;
            desc.destination_images = swapchain_image_handles;

            desc.src_mip_level = 0;
            desc.src_base_array_layer = 0;
            desc.dst_mip_level = 0;
            desc.dst_base_array_layer = 0;
            desc.width = swapchainDesc.width;
            desc.height = swapchainDesc.height;
            desc.depth = 1;
            desc.array_layer_count = 1;

            rg.read(rt_output_image, ogfx::ImageState::TransferSrc);
            for (const auto& image : swapchain_image_handles)
            {
                rg.write(image, ogfx::ImageState::TransferDst);
            }
        }
    );

    blit_node->set_pre_execute(
    [&](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd, orendergraph::RenderGraphNode<orendergraph::BlitContext>& node)
    {
        orendergraph::BlitContext::Desc& desc = node.descriptor();
        desc.destination_image_index = imageIndex;

        orendergraph::NodePreExecute<orendergraph::BlitContext>::execute(graph, cmd, *blit_node);
    });

    graph.add_begin_frame_transition(rt_output_image, ogfx::ImageState::Common);
    for (const auto& image : swapchain_image_handles)
    {
        graph.add_end_frame_transition(image, ogfx::ImageState::Present);
    }


    graph.compile();

    while (!window.should_close())
    {
        window.poll_events();

        Frame& frame = frames[current_frame];

        frame.in_flight->wait();

        imageIndex = swapchain.acquire_next_image(*frame.image_available.get());

        frame.in_flight->reset();

        ogfx::CommandBuffer& cmd = *frame.command_buffer;
        graph.execute(cmd);

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