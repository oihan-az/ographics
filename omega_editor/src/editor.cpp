#include <omega_editor/editor.hpp>

#include <omega/core/math/angle.hpp>
#include <omega/core/math/quaternion.hpp>

#include <omega/engine/engine.hpp>

#include <omega/transform/transform.hpp>
#include <omega/camera/camera.hpp>
#include <omega/lighting/directional_light.hpp>
#include <omega/lighting/point_light.hpp>
#include <omega/lighting/spot_light.hpp>

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
    : m_engine(engine), m_camera_viewport(engine.components())
{
}

void Editor::initialize()
{
    create_editor_camera();
    register_component_editors();
}

void Editor::render()
{
    // Fullscreen shortcut
    if (ImGui::IsKeyPressed(ImGuiKey_F11, false))
    {
        m_camera_viewport.toggle_fullscreen();
        m_viewport_toolbar.show();
    }

    // Editor panels
    if (!m_camera_viewport.fullscreen())
    {
        m_hierarchy_panel.render(m_left_slot, m_engine, m_selected_entity, m_camera_entity);

        m_inspection_panel.render(m_right_slot, m_engine, m_selected_entity, m_component_editors);
    }

    // Viewport layout
    update_camera_viewport();

    if (!m_camera_viewport.fullscreen())
        draw_viewport_outline(m_camera_viewport.rect());

    // Viewport toolbar
    const ViewportToolbarAction action = m_viewport_toolbar.render(m_camera_viewport);
    if (action == ViewportToolbarAction::ToggleFullscreen)
    {
        m_camera_viewport.toggle_fullscreen();
        m_viewport_toolbar.show();
    }

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

    m_camera_viewport.set_camera(m_camera_entity);
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

            changed |= ImGui::DragFloat("Aspect Ratio", &proj.aspect_ratio, 0.01f, 0.01f);
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
    const omega::CameraData& camera_data = m_engine.cameras().get_camera_data(m_camera_entity);
    m_transform_gizmo.render(m_camera_viewport, m_selected_entity, camera_data.view, camera_data.projection,
                             m_engine.components(), m_engine.transforms(), m_engine.hierarchy());
}

} // namespace omega_editor