#include <omega_editor/editor.hpp>

#include <omega/core/math/angle.hpp>
#include <omega/core/math/quaternion.hpp>

#include <omega/transform/transform.hpp>
#include <omega/camera/camera.hpp>
#include <omega/lighting/directional_light.hpp>
#include <omega/lighting/point_light.hpp>
#include <omega/lighting/spot_light.hpp>

#include <imgui.h>

namespace omega_editor
{

Editor::Editor(omega::Engine& engine) : m_engine(engine) {}

void Editor::initialize()
{
    register_component_editors();
}

void Editor::render()
{
    m_hierarchy_panel.render(m_engine, m_selected_entity);
    m_inspection_panel.render(m_engine, m_selected_entity, m_component_editors);
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

} // namespace omega_editor