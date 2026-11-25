#include <omega/engine/engine.hpp>

#include <omega/assets/model_importer.hpp>
#include <omega/model/model_system.hpp>
#include <omega/lighting/directional_light.hpp>
#include <omega/lighting/point_light.hpp>
#include <omega/lighting/spot_light.hpp>
#include <omega/atmosphere/atmosphere.hpp>
#include <omega/graphics/pipelines/raster_pipeline/features/to_swapchain_feature.hpp>
#include <omega/graphics/pipelines/raster_pipeline/features/auto_exposure_feature.hpp>

#include <omega_editor/ui/imgui_layer.hpp>
#include <omega_editor/rendering/ui_pipeline.hpp>
#include <omega_editor/editor.hpp>

#include <imgui.h>
#include <ImGuizmo.h>

int main()
{
    omega::EngineDesc desc;
    desc.application_name = "Omega Editor";
    desc.window = 
    {
        .width = 1280,
        .height = 720,
        .title = desc.application_name,
        .resizable = true,
        .fullscreen = false,
    };
    desc.max_frames_in_flight = 2;

    omega::Engine engine(desc);

    engine.initialize();

    // Load scene
    omega::ModelImporter importer;
    omega::ModelSystem models(engine.entities(), engine.hierarchy(), engine.components(), engine.graphics(), importer);
    const std::filesystem::path asset_dir = OMEGA_ASSET_DIR;
    const std::filesystem::path model_path = asset_dir / "models/sponza_glTF/Sponza.gltf";
    const omega::ModelHandle model = models.load(model_path);
    omega::Entity sponza;

    if (!models.instantiate(model, sponza))
        throw std::runtime_error("Failed to instantiate");

    // Create directional light
    omega::Entity directional_light_entity = engine.entities().create_entity();
    {
        omega::Transform transform;
        transform.rotation = omega::Quaternion{0.70710678f, -0.2, 0.0f, 0.0f};
        engine.components().add(directional_light_entity, transform);
    
        omega::DirectionalLight directional_light;
        directional_light.color = {0.9f, 0.85f, 0.8f};
        directional_light.intensity = 2.0f;
        engine.components().add(directional_light_entity, directional_light);

        omega::Atmosphere atmosphere{};
        engine.components().add(directional_light_entity, atmosphere);
    }

    // Create point light
    omega::Entity point_light_entity = engine.entities().create_entity();
    {
        omega::Transform transform;
        transform.position = {6.0f, 1.0f, 0.0f};
        engine.components().add(point_light_entity, transform);
    
        omega::PointLight point_light;
        point_light.color = {1.0f, 0.2f, 0.2f};
        point_light.intensity = 5.0f;
        point_light.range = 8.0f;
        engine.components().add(point_light_entity, point_light);
    }

    // Create spot light
    omega::Entity spot_light_entity = engine.entities().create_entity();
    {
        omega::Transform transform;
        transform.position = {5.0f, 1.4f, 0.0f};
        transform.rotation = omega::Quaternion{1.0f, 0.0f, 0.0f, 0.0f};
        engine.components().add(spot_light_entity, transform);
    
        omega::SpotLight spot_light;
        spot_light.color = {0.2f, 1.0f, 0.2f};
        spot_light.intensity = 30.0f;
        spot_light.range = 100.0f;
        spot_light.inner_angle = omega::radians(20.0f);
        spot_light.outer_angle = omega::radians(35.0f);
        engine.components().add(spot_light_entity, spot_light);
    }

    // Editor
    omega_editor::ImGuiLayer imgui_layer;
    omega_editor::Editor editor(engine);
    editor.initialize();

    // Renderer
    omega_editor::UIPipelineDesc pipeline_desc
    {
        editor.camera_viewport().camera().entity,
        editor.camera_viewport().camera().type,
        directional_light_entity,
        imgui_layer,
        engine.components(),
        engine.events(),
        engine.transforms(),
        engine.cameras(),
        engine.platform(),
        engine.graphics(),
    };

    omega_editor::UIPipeline& pipeline = engine.graphics().renderer().create_pipeline<omega_editor::UIPipeline>();
    engine.graphics().renderer().set_active_pipeline(0);
    engine.graphics().renderer().setup<omega_editor::UIPipeline>(pipeline_desc);
    engine.graphics().renderer().compile();

    using Clock = std::chrono::steady_clock;
    auto previous_frame_time = Clock::now();

    while (!engine.should_close())
    {
        if (!engine.start_frame())
            continue;

        const auto current_frame_time = Clock::now();
        const float delta_time = std::chrono::duration<float>(current_frame_time - previous_frame_time).count();
        previous_frame_time = current_frame_time;
        pipeline.m_auto_exposure_feature->set_delta_time(delta_time);

        engine.update();

        {
            omega::ScopedCpuTimer timer(engine.performance(), "Editor");

            imgui_layer.begin_frame();
            ImGuizmo::BeginFrame();

            editor.render();

            if (editor.camera_changed())
                pipeline.set_camera(editor.camera_viewport().camera().entity, editor.camera_viewport().camera().type);

            imgui_layer.end_frame();

            const omega_editor::ViewportRect& rect = editor.camera_viewport().rect();
            omega::RenderRegion region{static_cast<uint32_t>(rect.position.x), static_cast<uint32_t>(rect.position.y),
                                       static_cast<uint32_t>(rect.size.x), static_cast<uint32_t>(rect.size.y)};
            pipeline.set_viewport(region);
        }

        engine.render();

        engine.end_frame();
    }

    engine.wait();

    return 0;
}