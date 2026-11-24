#include <omega_editor/editor.hpp>

#include <omega/core/math/angle.hpp>
#include <omega/core/math/quaternion.hpp>

#include <omega/engine/engine.hpp>

#include <omega/transform/transform.hpp>
#include <omega/camera/camera.hpp>
#include <omega/lighting/directional_light.hpp>
#include <omega/lighting/point_light.hpp>
#include <omega/lighting/spot_light.hpp>
#include <omega/atmosphere/atmosphere.hpp>

#include <imgui.h>

namespace
{

void draw_viewport_outline(const omega_editor::ViewportRect& rect)
{
    if (rect.size.x <= 0.0f || rect.size.y <= 0.0f)
        return;

    const ImVec2 min = rect.position;
    const ImVec2 max(rect.position.x + rect.size.x, rect.position.y + rect.size.y);
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();

    draw_list->AddRect(min, max, IM_COL32(140, 150, 170, 150), 0.0f, 0, 1.0f);
}

} // namespace

namespace omega_editor
{

Editor::Editor(omega::Engine& engine)
    : m_engine(engine), m_camera_viewport(engine.components()), m_viewport_toolbar(engine.components(), m_camera_entity)
{
}

void Editor::initialize()
{
    create_editor_camera();

    register_component_editors();

    m_hierarchy_panel.set_content(m_hierarchy_content);
    m_properties_panel.add_content(m_inspection_content);
    m_properties_panel.add_content(m_editor_camera_content);
    m_properties_panel.add_content(m_performance_content);
}

void Editor::render()
{
    m_camera_changed = validate_camera_selection();

    // Fullscreen shortcut
    if (ImGui::IsKeyPressed(ImGuiKey_F11, false))
    {
        m_camera_viewport.toggle_fullscreen();
        m_viewport_toolbar.show();
    }

    // Editor panels
    if (!m_camera_viewport.fullscreen())
    {
        m_hierarchy_panel.render();
        m_properties_panel.render();
    }

    // Viewport toolbar
    const ViewportToolbarAction action = m_viewport_toolbar.render(m_camera_viewport, m_transform_gizmo);
    switch (action)
    {
    case ViewportToolbarAction::ToggleFullscreen:
    {
        m_camera_viewport.toggle_fullscreen();
        m_viewport_toolbar.show();
        break;
    }
    case ViewportToolbarAction::CameraChanged:
    {
        m_camera_changed = true;
        break;
    }
    case ViewportToolbarAction::None:
        break;
    }

    update_camera_viewport();

    if (!m_camera_viewport.fullscreen())
        draw_viewport_outline(m_camera_viewport.rect());

    update_editor_camera();

    if (m_camera_viewport.camera().entity == m_camera_entity)
        render_transform_gizmo();
}

omega::Entity Editor::editor_camera() const
{
    return m_camera_entity;
}

const CameraViewport& Editor::camera_viewport() const
{
    return m_camera_viewport;
}

bool Editor::camera_changed() const
{
    return m_camera_changed;
}

void Editor::create_editor_camera() 
{
    m_camera_entity = m_engine.entities().create_entity();

    omega::Transform transform;
    transform.position = {0.0f, 2.0f, 0.0f};
    transform.rotation = omega::Quaternion{0.70710678f, 0.0f, -0.70710678f, 0.0f};

    m_engine.components().add(m_camera_entity, transform);

    omega::PerspectiveCamera camera;
    camera.proj.aspect_ratio = m_camera_viewport.aspect_ratio();
    camera.proj.field_of_view = omega::radians(60.0f);
    camera.proj.near_plane = 0.1f;
    camera.proj.far_plane = 100.0f;

    m_engine.components().add(m_camera_entity, camera);

    omega::OrthogonalCamera orthographic;
    orthographic.proj.left = -8.0f;
    orthographic.proj.right = 8.0f;
    orthographic.proj.bottom = -4.5f;
    orthographic.proj.top = 4.5f;
    orthographic.proj.near_plane = 0.1f;
    orthographic.proj.far_plane = 100.0f;

    m_engine.components().add(m_camera_entity, orthographic);

    m_camera_viewport.set_camera({m_camera_entity, omega::CameraType::Perspective});
}

bool Editor::validate_camera_selection()
{
    const CameraSelection selection = m_camera_viewport.camera();

    if (selection.entity == m_camera_entity && selection.type == omega::CameraType::Perspective)
        return false;

    auto& components = m_engine.components();

    bool valid = m_engine.entities().is_alive(selection.entity);

    if (valid)
    {
        switch (selection.type)
        {
        case omega::CameraType::Perspective:
            valid = components.has<omega::PerspectiveCamera>(selection.entity);
            break;

        case omega::CameraType::Orthographic:
            valid = components.has<omega::OrthogonalCamera>(selection.entity);
            break;

        default:
            valid = false;
            break;
        }
    }

    if (valid)
        return false;

    m_camera_viewport.set_camera({m_camera_entity, omega::CameraType::Perspective});

    return true;
}

void Editor::register_component_editors()
{
    m_component_editors.register_component<omega::Transform>
    (
        "Transform",
        [](omega::Transform& transform)
        {
            bool changed = false;

            changed |= ImGui::DragFloat3("Position", &transform.position.x, 0.1f);

            omega::Vector3 rotation = omega::degrees(omega::euler_angles(transform.rotation));
            if (ImGui::DragFloat3("Rotation", &rotation.x, 0.1f))
            {
                transform.rotation = glm::quat(omega::radians(rotation));
                changed = true;
            }

            static bool uniform_scale = true;

            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Scale");
            ImGui::SameLine();

            const float button_size = ImGui::GetFrameHeight();

            if (ImGui::Button(uniform_scale ? "U" : "D", ImVec2(button_size, button_size)))
            {
                uniform_scale = !uniform_scale;
            
                if (uniform_scale)
                {
                    transform.scale.z = transform.scale.y = transform.scale.x;
                }

                changed = true;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Uniform Scale");

            ImGui::SameLine();

            if (uniform_scale)
            {
                float scale = transform.scale.x;

                if (ImGui::DragFloat("##Scale", &scale, 0.1f))
                {
                    transform.scale = omega::Vector3(scale);
                    changed = true;
                }
            }
            else
            {
                changed |= ImGui::DragFloat3("##Scale", &transform.scale.x, 0.1f);
            }

            return changed;
        }
    );

    m_component_editors.register_component<omega::PerspectiveCamera>
    (
        "Perspective Camera",
        [](omega::PerspectiveCamera& camera)
        {
            bool changed = false;
            auto& proj = camera.proj;

            float fov = omega::degrees(proj.field_of_view);

            if (ImGui::DragFloat("Field of View", &fov, 0.5f, 1.0f, 179.0f))
            {
                proj.field_of_view = omega::radians(fov);
                changed = true;
            }

            changed |= ImGui::DragFloat("Aspect Ratio", &proj.aspect_ratio, 0.01f, 0.01f, 10.0f);
            changed |= ImGui::DragFloat("Near Plane", &proj.near_plane, 0.01f, 0.001f);
            changed |= ImGui::DragFloat("Far Plane", &proj.far_plane, 1.0f, proj.near_plane);

            return changed;
        }
    );

    m_component_editors.register_component<omega::OrthogonalCamera>
    (
        "Orthographic Camera",
        [](omega::OrthogonalCamera& camera)
        {
            bool changed = false;
            auto& proj = camera.proj;

            changed |= ImGui::DragFloat("Left", &proj.left, 0.1f);
            changed |= ImGui::DragFloat("Right", &proj.right, 0.1f);
            changed |= ImGui::DragFloat("Bottom", &proj.bottom, 0.1f);
            changed |= ImGui::DragFloat("Top", &proj.top, 0.1f);
            changed |= ImGui::DragFloat("Near Plane", &proj.near_plane, 0.01f);
            changed |= ImGui::DragFloat("Far Plane", &proj.far_plane, 1.0f);

            return changed;
        }
    );

    m_component_editors.register_component<omega::DirectionalLight>
    (
        "Directional Light",
        [](omega::DirectionalLight& light)
        {
            bool changed = false;

            changed |= ImGui::ColorEdit3("Color", &light.color.x);
            changed |= ImGui::DragFloat("Intensity", &light.intensity, 0.1f, 0.0f);

            return changed;
        }
    );

    m_component_editors.register_component<omega::PointLight>
    (
        "Point Light",
        [](omega::PointLight& light)
        {
            bool changed = false;

            changed |= ImGui::ColorEdit3("Color", &light.color.x);
            changed |= ImGui::DragFloat("Intensity", &light.intensity, 0.1f, 0.0f);
            changed |= ImGui::DragFloat("Range", &light.range, 0.1f, 0.0f);

            return changed;
        }
    );

    m_component_editors.register_component<omega::SpotLight>
    (
        "Spot Light",
        [](omega::SpotLight& light)
        {
            bool changed = false;

            changed |= ImGui::ColorEdit3("Color", &light.color.x);
            changed |= ImGui::DragFloat("Intensity", &light.intensity, 0.1f, 0.0f);
            changed |= ImGui::DragFloat("Range", &light.range, 0.1f, 0.0f);

            float inner_angle = omega::degrees(light.inner_angle);
            float outer_angle = omega::degrees(light.outer_angle);

            if (ImGui::DragFloat("Inner Angle", &inner_angle, 0.5f, 0.0f, 180.0f))
            {
                light.inner_angle = omega::radians(inner_angle);
                changed = true;
            }

            if (ImGui::DragFloat("Outer Angle", &outer_angle, 0.5f, 0.0f, 180.0f))
            {
                light.outer_angle = omega::radians(outer_angle);
                changed = true;
            }

            return changed;
        }
    );

    m_component_editors.register_component<omega::Atmosphere>
    (
        "Atmosphere",
        [](omega::Atmosphere& atmosphere)
        {
            bool changed = false;

            constexpr float max_float = std::numeric_limits<float>::max();

            // Planet
            changed |= ImGui::DragFloat("Planet Radius", &atmosphere.planet_radius, 100.0f, 1.0f, max_float);
            changed |= ImGui::DragFloat("Atmosphere Radius", &atmosphere.atmosphere_radius, 100.0f, atmosphere.planet_radius + 1.0f, max_float);

            // Keep the atmosphere larger than the planet
            if (atmosphere.atmosphere_radius <= atmosphere.planet_radius)
            {
                atmosphere.atmosphere_radius = atmosphere.planet_radius + 1.0f;
                changed = true;
            }

            // Atmospheric density
            changed |= ImGui::DragFloat("Rayleigh Scale Height", &atmosphere.rayleigh_scale_height, 10.0f, 1.0f, max_float);
            changed |= ImGui::DragFloat("Mie Scale Height", &atmosphere.mie_scale_height, 10.0f, 1.0f, max_float);

            // Scattering coefficients
            changed |= ImGui::DragFloat3("Rayleigh Scattering", &atmosphere.rayleigh_scattering.x, 1e-7f, 0.0f, max_float, "%.8f");
            changed |= ImGui::DragFloat("Mie Scattering", &atmosphere.mie_scattering, 1e-7f, 0.0f, max_float, "%.8f");
            changed |= ImGui::SliderFloat("Mie Anisotropy", &atmosphere.mie_anisotropy, -0.99f, 0.99f);
            changed |= ImGui::DragFloat("Ground Offset", &atmosphere.ground_offset, 0.1f, 0.0f, max_float);

            return changed;
        }
    );
}

void Editor::update_editor_camera()
{
    m_editor_camera_controller.update(ImGui::GetIO().DeltaTime, m_camera_viewport, m_editor_camera_controls, m_engine, m_camera_entity, m_selected_entity);
}

void Editor::update_camera_viewport()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    const ImVec2 origin = viewport->WorkPos;
    const ImVec2 size = viewport->WorkSize;

    ViewportRect available{origin, size};

    if (!m_camera_viewport.fullscreen())
    {
        constexpr float padding = 8.0f;

        const PanelBounds& left = m_left_slot.bounds();
        const PanelBounds& right = m_right_slot.bounds();

        const float x_min = left.x + left.width + padding;
        const float x_max = right.x - padding;

        available.position.x = x_min;
        available.size.x = std::max(0.0f, x_max - x_min);
    }

    m_camera_viewport.update(available);
}

void Editor::render_transform_gizmo() 
{
    const CameraSelection selection = m_camera_viewport.camera();

    const omega::CameraData& camera_data = m_engine.cameras().get_camera_data(selection.entity, selection.type);
    m_transform_gizmo.render(m_camera_viewport, m_selected_entity, camera_data.view, camera_data.projection,
                             m_engine.components(), m_engine.transforms(), m_engine.hierarchy(),
                             m_editor_camera_controller.is_navigating());
}

} // namespace omega_editor