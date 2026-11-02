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
#include <ogfx/ray_tracing_pipeline.hpp>

#include <ogfx/acc_struct.hpp>
#include <ogfx/acc_struct_geometry.hpp>
#include <ogfx/acc_struct_instance.hpp>

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




    // Command pool
    ogfx::Queue& graphicsQueue = device.queue(ogfx::QueueType::Graphics);

    ogfx::CommandPoolDesc commandPoolDesc;
    commandPoolDesc.transient = false;
    commandPoolDesc.resettable = true;

    ogfx::CommandPool commandPool(device, graphicsQueue, commandPoolDesc);

    ogfx::CommandBuffer commandBuffer(commandPool);




    SceneGeometry geometry;
    generate_sphere(geometry, 64, 32, 32);

    // Vertex buffer
    ogfx::BufferDesc vertex_buffer_desc;
    vertex_buffer_desc.size = sizeof(Vertex) * geometry.vertices.size();
    vertex_buffer_desc.usage = ogfx::BufferUsage::Vertex | 
                               ogfx::BufferUsage::Storage | 
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

    // Index buffer
    ogfx::BufferDesc index_buffer_desc;
    index_buffer_desc.size = sizeof(Index) * geometry.indices.size();
    index_buffer_desc.usage = ogfx::BufferUsage::Index | 
                              ogfx::BufferUsage::Storage | 
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




    ogfx::ImageDesc output_image_desc{};
    output_image_desc.width = windowDesc.width;
    output_image_desc.height = windowDesc.height;
    output_image_desc.format = ogfx::Format::RGBA32_FLOAT;
    output_image_desc.usage = ogfx::ImageUsage::Storage | ogfx::ImageUsage::TransferSrc;

    ogfx::Image rt_output_image(physicalDevice, device, output_image_desc);
    rt_output_image.allocate_memory();
    rt_output_image.bind_memory();

    ogfx::ImageViewDesc image_view_desc;
    image_view_desc.image = &rt_output_image;

    ogfx::ImageView image_view(image_view_desc);




    commandBuffer.begin();

    std::vector<ogfx::BLAS> bottom_level_structures;
    bottom_level_structures.reserve(geometry.clusters.size());

    for (const MeshCluster& cluster : geometry.clusters)
    {
        ogfx::BLASDesc blas_desc{};

        ogfx::AccelerationStructureTriangleGeometry triangle_geometry{};
        triangle_geometry.vertex_buffer = &vertex_buffer;
        triangle_geometry.vertex_offset = 0;
        triangle_geometry.vertex_stride = sizeof(Vertex);
        triangle_geometry.vertex_count = static_cast<uint32_t>(geometry.vertices.size());
        triangle_geometry.vertex_format = ogfx::Format::RGB32_FLOAT;

        triangle_geometry.index_buffer = &index_buffer;
        triangle_geometry.index_offset = static_cast<uint64_t>(cluster.first_index) * sizeof(Index);
        triangle_geometry.index_count = cluster.index_count;
        triangle_geometry.index_type = ogfx::IndexType::UInt32;
        triangle_geometry.primitive_count = cluster.index_count / 3;

        blas_desc.triangle_geometries.push_back(triangle_geometry);

        bottom_level_structures.emplace_back(physicalDevice, device, blas_desc);

        ogfx::AccelerationStructureBuildInfo blas_build_info;
        blas_build_info.destination = &bottom_level_structures.back();
        commandBuffer.build_acceleration_structure(blas_build_info);
    }




    ogfx::MemoryBarrierInfo blas_to_tlas_barrier{};
    blas_to_tlas_barrier.src_stages = ogfx::PipelineStage::AccelerationStructureBuild;
    blas_to_tlas_barrier.dst_stages = ogfx::PipelineStage::AccelerationStructureBuild;
    blas_to_tlas_barrier.src_access = ogfx::AccessFlags::AccelerationStructureWrite;
    blas_to_tlas_barrier.dst_access = ogfx::AccessFlags::AccelerationStructureRead;

    commandBuffer.memory_barrier(blas_to_tlas_barrier);




    std::vector<ogfx::AccelerationStructureInstance> instances;
    instances.reserve(bottom_level_structures.size());

    for (uint32_t i = 0; i < bottom_level_structures.size(); ++i)
    {
        ogfx::AccelerationStructureInstance instance{};
        instance.acceleration_structure = &bottom_level_structures[i];
        instance.custom_index = i;

        instances.push_back(instance);
    }

    ogfx::BufferDesc instance_buffer_desc{};
    instance_buffer_desc.size = ogfx::TLAS::instance_size() * instances.size();
    instance_buffer_desc.usage = ogfx::BufferUsage::AccelerationStructureBuildInput | ogfx::BufferUsage::ShaderDeviceAddress;
    instance_buffer_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

    ogfx::Buffer instance_buffer(physicalDevice, device, instance_buffer_desc);

    instance_buffer.allocate_memory();
    instance_buffer.bind_memory();

    ogfx::TLAS::write_instances_to_buffer(instances, instance_buffer);

    ogfx::TLASDesc tlas_desc{};

    ogfx::AccelerationStructureInstanceGeometry instance_geometry{};
    instance_geometry.instance_count = static_cast<uint32_t>(instances.size());
    instance_geometry.instance_buffer = &instance_buffer;
    instance_geometry.instance_offset = 0;

    tlas_desc.instance_geometries.push_back(instance_geometry);

    ogfx::TLAS top_level_structure(physicalDevice, device, tlas_desc);

    ogfx::AccelerationStructureBuildInfo tlas_build_info;
    tlas_build_info.destination = &top_level_structure;
    commandBuffer.build_acceleration_structure(tlas_build_info);




    ogfx::MemoryBarrierInfo tlas_to_shaders_barrier{};
    tlas_to_shaders_barrier.src_stages = ogfx::PipelineStage::AccelerationStructureBuild;
    tlas_to_shaders_barrier.dst_stages = ogfx::PipelineStage::RayTracingShader;
    tlas_to_shaders_barrier.src_access = ogfx::AccessFlags::AccelerationStructureWrite;
    tlas_to_shaders_barrier.dst_access = ogfx::AccessFlags::AccelerationStructureRead;

    commandBuffer.memory_barrier(tlas_to_shaders_barrier);

    commandBuffer.end();

    {
        ogfx::Fence buildFence(device);
        buildFence.reset();

        ogfx::SubmitDesc submitDesc;
        submitDesc.command_buffers.push_back(&commandBuffer);
        submitDesc.fence = &buildFence;

        graphicsQueue.submit(submitDesc);

        buildFence.wait();
    }




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

    ogfx::Shader raygen_shader(device, raygen_desc);
    ogfx::Shader miss_shader(device, miss_desc);
    ogfx::Shader closesthit_shader(device, closesthit_desc);




    ogfx::ResourceAllocator resource_allocator(device);

    ogfx::DescriptorPoolDesc pool_desc;
    pool_desc.max_sets = 4;
    pool_desc.sizes = {
        {ogfx::ResourceType::StorageTexture, 1},
        {ogfx::ResourceType::AccelerationStructure, 1},
        {ogfx::ResourceType::StorageBuffer, 3}
    };

    resource_allocator.create_pool(pool_desc);

    ogfx::ResourceBindingDesc tlas_binding;
    tlas_binding.binding = 0;
    tlas_binding.type = ogfx::ResourceType::AccelerationStructure;
    tlas_binding.array_count = 1;
    tlas_binding.stages = ogfx::ShaderStageFlags::RayGen | ogfx::ShaderStageFlags::ClosestHit;

    ogfx::ResourceBindingDesc output_image_binding;
    output_image_binding.binding = 1;
    output_image_binding.type = ogfx::ResourceType::StorageTexture;
    output_image_binding.array_count = 1;
    output_image_binding.stages = ogfx::ShaderStageFlags::RayGen;

    ogfx::ResourceBindingDesc vertices_binding;
    vertices_binding.binding = 2;
    vertices_binding.type = ogfx::ResourceType::StorageBuffer;
    vertices_binding.array_count = 1;
    vertices_binding.stages = ogfx::ShaderStageFlags::ClosestHit;

    ogfx::ResourceBindingDesc indices_binding;
    indices_binding.binding = 3;
    indices_binding.type = ogfx::ResourceType::StorageBuffer;
    indices_binding.array_count = 1;
    indices_binding.stages = ogfx::ShaderStageFlags::ClosestHit;

    ogfx::ResourceBindingDesc clusters_binding;
    clusters_binding.binding = 4;
    clusters_binding.type = ogfx::ResourceType::StorageBuffer;
    clusters_binding.array_count = 1;
    clusters_binding.stages = ogfx::ShaderStageFlags::ClosestHit;

    ogfx::ResourceSetLayoutDesc rt_set_layout_desc;
    rt_set_layout_desc.bindings = 
    {
        tlas_binding, 
        output_image_binding,
        vertices_binding,
        indices_binding,
        clusters_binding
    };

    ogfx::ResourceSetLayout rt_set_layout(device, rt_set_layout_desc);

    ogfx::PipelineLayoutDesc rt_pipeline_layout_desc;
    rt_pipeline_layout_desc.set_layouts = {&rt_set_layout};
    rt_pipeline_layout_desc.inline_constant_ranges = {};

    ogfx::PipelineLayout rt_pipeline_layout(device, rt_pipeline_layout_desc);



    ogfx::ResourceSetDesc rt_set_desc;
    rt_set_desc.layout = &rt_set_layout;

    ogfx::ResourceSet rt_resource_set(device, rt_set_desc);
    rt_resource_set.allocate(resource_allocator);

    ogfx::ImageBinding rt_output_image_write;
    rt_output_image_write.binding = 1;
    rt_output_image_write.image_view = &image_view;
    rt_output_image_write.image_state = ogfx::ImageState::Common;

    ogfx::AccelerationStructureBinding rt_tlas_write;
    rt_tlas_write.binding = 0;
    rt_tlas_write.acceleration_structure = &top_level_structure;

    ogfx::BufferBinding rt_vertex_buffer_write;
    rt_vertex_buffer_write.binding = 2;
    rt_vertex_buffer_write.buffer = &vertex_buffer;
    rt_vertex_buffer_write.size = vertex_buffer_desc.size;

    ogfx::BufferBinding rt_index_buffer_write;
    rt_index_buffer_write.binding = 3;
    rt_index_buffer_write.buffer = &index_buffer;
    rt_index_buffer_write.size = index_buffer_desc.size;

    ogfx::BufferBinding rt_clusters_buffer_write;
    rt_clusters_buffer_write.binding = 4;
    rt_clusters_buffer_write.buffer = &cluster_buffer;
    rt_clusters_buffer_write.size = cluster_buffer_desc.size;

    ogfx::ResourceSetWriteInfo rt_write_info;
    rt_write_info.textures = {rt_output_image_write};
    rt_write_info.acceleration_structures = {rt_tlas_write};
    rt_write_info.buffers = 
    {
        rt_vertex_buffer_write, 
        rt_index_buffer_write,
        rt_clusters_buffer_write
    };

    rt_resource_set.write(rt_write_info);





    ogfx::RayTracingPipelineDesc pipeline_desc{};
    pipeline_desc.layout = &rt_pipeline_layout;
    pipeline_desc.max_recursion_depth = 1;
    pipeline_desc.raygen_shader = &raygen_shader;
    pipeline_desc.miss_shaders.push_back(&miss_shader);

    ogfx::RayTracingHitShaders cluster_hit_group{};
    cluster_hit_group.closest_hit_shader = &closesthit_shader;
    pipeline_desc.hit_shaders.push_back(cluster_hit_group);

    ogfx::RayTracingPipeline ray_tracing_pipeline(physicalDevice, device, pipeline_desc);




    uint32_t group_count = ray_tracing_pipeline.group_count();

    ogfx::BufferDesc sbt_buffer_desc;
    sbt_buffer_desc.size = ray_tracing_pipeline.sbt_size(group_count);
    sbt_buffer_desc.usage = ogfx::BufferUsage::ShaderBindingTable | ogfx::BufferUsage::ShaderDeviceAddress;
    sbt_buffer_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

    ogfx::Buffer sbt_buffer(physicalDevice, device, sbt_buffer_desc);

    sbt_buffer.allocate_memory();
    sbt_buffer.bind_memory();

    ray_tracing_pipeline.write_handles_to_buffer(0, group_count, sbt_buffer);




    ogfx::SwapchainDesc swapchainDesc;
    swapchainDesc.width = window.width();
    swapchainDesc.height = window.height();
    swapchainDesc.format = ogfx::Format::BGRA8_UNORM;
    swapchainDesc.present_mode = ogfx::PresentMode::FIFO;
    swapchainDesc.image_count = 3;

    ogfx::Swapchain swapchain(physicalDevice, device, surface, swapchainDesc);




    ogfx::Semaphore imageAvailable(device);
    ogfx::Semaphore renderFinished(device);

    ogfx::Fence inFlight(device);
    inFlight.reset();

    ogfx::Queue& presentQueue = device.queue(ogfx::QueueType::Present);

    while (!window.should_close())
    {
        window.poll_events();

        {
            const uint32_t imageIndex = swapchain.acquire_next_image(imageAvailable);

            commandBuffer.begin();

            ogfx::ImageTransitionInfo layout_to_general{};
            layout_to_general.image = &rt_output_image;
            layout_to_general.old_state = ogfx::ImageState::Undefined;
            layout_to_general.new_state = ogfx::ImageState::Common;
            commandBuffer.transition_image(layout_to_general);

            ogfx::PipelineBindInfo rt_bind_info;
            rt_bind_info.pipeline = &ray_tracing_pipeline;
            commandBuffer.bind_pipeline(rt_bind_info);

            ogfx::ResourceSetBindInfo resource_set_bind_info;
            resource_set_bind_info.set_index = 0;
            resource_set_bind_info.pipeline_type = ogfx::PipelineType::RayTracing;
            resource_set_bind_info.resource_set = &rt_resource_set;
            resource_set_bind_info.layout = &rt_pipeline_layout;

            commandBuffer.bind_resource_set(resource_set_bind_info);

            uint64_t sbt_address = sbt_buffer.device_address();

            ogfx::TraceRaysInfo trace_info{};
            trace_info.width = swapchainDesc.width;
            trace_info.height = swapchainDesc.height;
            trace_info.depth = 1;
            trace_info.sbt_buffer = &sbt_buffer;
            trace_info.pipeline = &ray_tracing_pipeline;

            commandBuffer.trace_rays(trace_info);

            ogfx::ImageTransitionInfo offscreen_to_src{};
            offscreen_to_src.image = &rt_output_image;
            offscreen_to_src.old_state = ogfx::ImageState::Common;
            offscreen_to_src.new_state = ogfx::ImageState::TransferSrc;
            commandBuffer.transition_image(offscreen_to_src);

            ogfx::ImageTransitionInfo swapchain_to_dst{};
            swapchain_to_dst.image = &swapchain.image(imageIndex);
            swapchain_to_dst.old_state = ogfx::ImageState::Undefined;
            swapchain_to_dst.new_state = ogfx::ImageState::TransferDst;
            commandBuffer.transition_image(swapchain_to_dst);

            ogfx::ImageToImageCopyInfo copy_info{};
            copy_info.src_image = &rt_output_image;
            copy_info.src_mip_level = 0;
            copy_info.src_base_array_layer = 0;

            copy_info.dst_image = &swapchain.image(imageIndex);
            copy_info.dst_mip_level = 0;
            copy_info.dst_base_array_layer = 0;

            copy_info.width = swapchainDesc.width;
            copy_info.height = swapchainDesc.height;
            copy_info.depth = 1;
            copy_info.array_layer_count = 1;

            commandBuffer.blit_image(copy_info);

            ogfx::ImageTransitionInfo swapchain_to_present{};
            swapchain_to_present.image = &swapchain.image(imageIndex);
            swapchain_to_present.old_state = ogfx::ImageState::TransferDst;
            swapchain_to_present.new_state = ogfx::ImageState::Present;
            swapchain_to_present.base_mip_level = 0;
            swapchain_to_present.mip_level_count = 1;
            swapchain_to_present.base_array_layer = 0;
            swapchain_to_present.array_layer_count = 1;
            commandBuffer.transition_image(swapchain_to_present);

            commandBuffer.end();

            ogfx::SubmitDesc submitDesc;
            submitDesc.command_buffers.push_back(&commandBuffer);

            submitDesc.wait_semaphores.push_back({&imageAvailable, ogfx::PipelineStage::RayTracingShader});
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