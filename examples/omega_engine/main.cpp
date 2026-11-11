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

#include <omega/core/event/event_system.hpp>
#include <omega/core/entity/entity_system.hpp>
#include <omega/core/hierarchy/hierarchy_system.hpp>
#include <omega/core/component/component_system.hpp>
#include <omega/transform/transform_system.hpp>
#include <omega/camera/camera_system.hpp>
#include <omega/mesh/mesh_system.hpp>
#include <omega/mesh/gpu_mesh_system.hpp>
#include <omega/renderable/renderable.hpp>

#include <omega/graphics/pipelines/raster_pipeline/raster_pipeline.hpp>

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
    windowDesc.title = "Omega Engine";
    windowDesc.resizable = true;
    windowDesc.fullscreen = false;

    oplatform::Window window(windowDesc);

    const auto extensions = runtime.required_instance_extensions();

    // Graphics framework setup
    ogfx::InstanceDesc instance_desc;
    instance_desc.application_name = "Omega Engine";
    instance_desc.engine_name = "Omega Engine";
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

    // Systems
    omega::EventSystem events;
    omega::EntitySystem entities(events);
    omega::HierarchySystem hierarchy(events, entities);
    omega::ComponentSystem components(events, entities);
    omega::TransformSystem transforms(events, components, hierarchy, entities);
    omega::CameraSystem cameras(events, components, hierarchy, entities, transforms);

    orenderer::Renderer renderer(physical_device, device, resource_allocator);

    omega::MeshSystem meshes(events);
    omega::GpuMeshSystem gpu_meshes(physical_device, device, renderer.resource_registry(), meshes, events);

    // Load meshes
    omega::MeshHandle triangle_mesh;
    {
        omega::MeshAsset mesh_asset{};

        ogfx::VertexBindingDesc vertex_binding{};
        vertex_binding.binding = 0;
        vertex_binding.stride = sizeof(omega::Vector3);
        vertex_binding.input_rate = ogfx::VertexInputRate::Vertex;

        ogfx::VertexAttributeDesc position_attribute{};
        position_attribute.location = 0;
        position_attribute.binding = 0;
        position_attribute.format = ogfx::Format::RGB32_FLOAT;
        position_attribute.offset = 0;

        mesh_asset.vertex_input.bindings = {vertex_binding};
        mesh_asset.vertex_input.attributes = {position_attribute};

        const std::vector<omega::Vector3> vertices = {{-0.5f, -0.5f, 0.0f}, {0.5f, -0.5f, 0.0f}, {0.0f, 0.5f, 0.0f}};

        mesh_asset.vertex_count = static_cast<uint32_t>(vertices.size());

        const auto* vertex_bytes = reinterpret_cast<const std::byte*>(vertices.data());

        mesh_asset.vertex_data.emplace_back(vertex_bytes, vertex_bytes + vertices.size() * sizeof(omega::Vector3));

        if (!meshes.create_mesh(mesh_asset, triangle_mesh))
            return -1;

        gpu_meshes.make_resident(triangle_mesh);
        gpu_meshes.synchronize();
    }

    // Create scene
    omega::Entity camera_entity = entities.create_entity();
    omega::Entity renderable_entity = entities.create_entity();
    {
        {
            omega::Transform transform;
            transform.position = {0.0f, 0.0f, 3.0f};
            components.add(camera_entity, transform);

            omega::PerspectiveCamera perspective_camera;
            perspective_camera.proj.aspect_ratio = static_cast<float>(swapchain_desc.width) / static_cast<float>(swapchain_desc.height);
            perspective_camera.proj.field_of_view = omega::radians(60.0f);
            perspective_camera.proj.near_plane = 0.1f;
            perspective_camera.proj.far_plane = 100.0f;
            components.add(camera_entity, perspective_camera);
        }

        {
            omega::Transform transform;
            components.add(renderable_entity, transform);

            omega::Renderable renderable;
            renderable.mesh = triangle_mesh;
            renderable.color = {1.0f, 0.2f, 0.1f};
            components.add(renderable_entity, renderable);
        }
    }

    // Renderer
    {
        orendergraph::ResourcePoolRegistry& registry = renderer.resource_registry();

        // Pipelines
        {
            std::vector<orendergraph::ResourceHandle<ogfx::Image>> swapchain_images;
            swapchain_images.resize(swapchain.image_count());
            for (uint32_t i = 0; i < swapchain.image_count(); i++)
                swapchain_images[i] = registry.import_resource(&swapchain.image(i));

            omega::RasterPipelineDesc pipeline_desc
            {
                camera_entity, 
                swapchain_desc.width,
                swapchain_desc.height,
                swapchain_images,
                &swapchainImageIndex,
                components,
                transforms,
                cameras,
                gpu_meshes,
            };

            renderer.create_pipeline<omega::RasterPipeline>();
            renderer.set_active_pipeline(0);
            renderer.setup<omega::RasterPipeline>(pipeline_desc);
            renderer.compile();
        }
    }

    while (!window.should_close())
    {
        window.poll_events();

        Frame& frame = frames[current_frame];
        frame.in_flight->wait();

        swapchainImageIndex = swapchain.acquire_next_image(*frame.image_available.get());

        // Update systems
        events.swap_all_buffers();
        entities.update();
        hierarchy.update();
        components.update();
        transforms.update();
        cameras.update();
        gpu_meshes.synchronize();

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