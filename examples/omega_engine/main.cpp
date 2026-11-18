#include <oplatform/runtime.hpp>
#include <oplatform/window.hpp>

#include <ogfx/instance.hpp>
#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/resource_allocator.hpp>
#include <ogfx/swapchain.hpp>

#include <omega/core/event/event_system.hpp>
#include <omega/core/entity/entity_system.hpp>
#include <omega/core/hierarchy/hierarchy_system.hpp>
#include <omega/core/component/component_system.hpp>
#include <omega/transform/transform_system.hpp>
#include <omega/camera/camera_system.hpp>
#include <omega/mesh/mesh_system.hpp>
#include <omega/mesh/gpu_mesh_system.hpp>
#include <omega/texture/texture_system.hpp>
#include <omega/texture/gpu_texture_system.hpp>
#include <omega/renderable/renderable.hpp>
#include <omega/assets/model_importer.hpp>
#include <omega/model/model_system.hpp>
#include <omega/lighting/directional_light.hpp>
#include <omega/lighting/point_light.hpp>
#include <omega/lighting/spot_light.hpp>

#include <omega/graphics/graphics_system.hpp>
#include <omega/graphics/pipelines/raster_pipeline/raster_pipeline.hpp>

#include <vector>

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

    // Graphics framework setup
    ogfx::InstanceDesc instance_desc;
    instance_desc.application_name = "Omega Engine";
    instance_desc.engine_name = "Omega Engine";
    instance_desc.extensions = ogfx::InstanceExtension::DebugUtils | runtime.required_instance_extensions();

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

    // Systems
    omega::EventSystem events;
    omega::EntitySystem entities(events);
    omega::HierarchySystem hierarchy(events, entities);
    omega::ComponentSystem components(events, entities);
    omega::TransformSystem transforms(events, components, hierarchy, entities);
    omega::CameraSystem cameras(events, components, hierarchy, entities, transforms);
    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    omega::GraphicsSystem graphics(physical_device, device, resource_allocator, swapchain, MAX_FRAMES_IN_FLIGHT, events);
    graphics.initialize();

    // Load scene
    omega::TextureHandle missing_texture = graphics.missing_texture();
    omega::TextureViewHandle missing_texture_view = graphics.missing_texture_view();

    omega::ModelImporter importer;
    omega::ModelSystem models(entities, hierarchy, components, graphics.meshes(), graphics.textures(), importer);
    const std::filesystem::path asset_dir = OMEGA_ASSET_DIR;
    const std::filesystem::path model_path = asset_dir / "models/sponza_glTF/Sponza.gltf";
    const omega::ModelHandle model = models.load(model_path);
    omega::Entity sponza;

    if (!models.instantiate(model, missing_texture, sponza))
        throw std::runtime_error("Failed to instantiate");

    // Create camera
    omega::Entity camera_entity = entities.create_entity();
    {
        omega::Transform transform;
        transform.position = {0.0f, 2.0f, 0.0f};
        transform.rotation = omega::Quaternion{0.70710678f, 0.0f, -0.70710678f, 0.0f};
        components.add(camera_entity, transform);

        omega::PerspectiveCamera perspective_camera;
        perspective_camera.proj.aspect_ratio = static_cast<float>(swapchain_desc.width) / static_cast<float>(swapchain_desc.height);
        perspective_camera.proj.field_of_view = omega::radians(60.0f);
        perspective_camera.proj.near_plane = 0.1f;
        perspective_camera.proj.far_plane = 100.0f;
        components.add(camera_entity, perspective_camera);
    }

    // Create directional light
    omega::Entity directional_light_entity = entities.create_entity();
    {
        omega::Transform transform;
        transform.rotation = omega::Quaternion{0.70710678f, -0.2, 0.0f, 0.0f};
        components.add(directional_light_entity, transform);
    
        omega::DirectionalLight directional_light;
        directional_light.color = {0.2f, 1.0f, 0.2f};
        directional_light.intensity = 1.0f;
        components.add(directional_light_entity, directional_light);
    }

    // Create point light
    omega::Entity point_light_entity = entities.create_entity();
    {
        omega::Transform transform;
        transform.position = {6.0f, 1.0f, 0.0f};
        components.add(point_light_entity, transform);
    
        omega::PointLight point_light;
        point_light.color = {1.0f, 0.2f, 0.2f};
        point_light.intensity = 5.0f;
        point_light.range = 8.0f;
        components.add(point_light_entity, point_light);
    }

    // Create spot light
    omega::Entity spot_light_entity = entities.create_entity();
    {
        omega::Transform transform;
        transform.position = {5.0f, 1.4f, 0.0f};
        transform.rotation = omega::Quaternion{1.0f, 0.0f, 0.0f, 0.0f};
        components.add(spot_light_entity, transform);
    
        omega::SpotLight spot_light;
        spot_light.color = {0.2f, 0.2f, 1.0f};
        spot_light.intensity = 8.0f;
        spot_light.range = 100.0f;
        spot_light.inner_angle = omega::radians(20.0f);
        spot_light.outer_angle = omega::radians(35.0f);
        components.add(spot_light_entity, spot_light);
    }

    // Renderer
    {
        // Pipelines
        {
            omega::RasterPipelineDesc pipeline_desc
            {
                camera_entity, 
                components,
                events,
                transforms,
                cameras,
                graphics,
            };

            graphics.renderer().create_pipeline<omega::RasterPipeline>();
            graphics.renderer().set_active_pipeline(0);
            graphics.renderer().setup<omega::RasterPipeline>(pipeline_desc);
            graphics.renderer().compile();
        }
    }

    while (!window.should_close())
    {
        window.poll_events();

        graphics.start_frame();

        if (graphics.needs_swapchain_recreation())
        {
            const uint32_t width = window.width();
            const uint32_t height = window.height();

            if (width == 0 || height == 0)
                continue;

            graphics.recreate_swapchain(width, height);
            continue;
        }

        events.swap_all_buffers();

        entities.update();
        hierarchy.update();
        components.update();
        transforms.update();
        cameras.update();
        graphics.update();

        graphics.render();

        graphics.end_frame();
    }

    device.wait_idle();

    return 0;
}