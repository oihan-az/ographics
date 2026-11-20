#include <omega_editor/gizmos/transform_gizmo.hpp>

#include <omega/core/component/component_system.hpp>
#include <omega/core/hierarchy/hierarchy_system.hpp>
#include <omega/transform/transform.hpp>
#include <omega/transform/transform_system.hpp>

#include <omega/core/math/matrix4.hpp>
#include <omega/core/math/vector4.hpp>
#include <omega/core/math/vector3.hpp>
#include <omega/core/math/quaternion.hpp>

#include <imgui.h>

namespace omega_editor
{

void TransformGizmo::handle_shortcuts()
{
    ImGuiIO& io = ImGui::GetIO();

    if (io.WantTextInput || ImGuizmo::IsUsing())
        return;

    if (ImGui::IsKeyPressed(ImGuiKey_W, false))
        m_operation = ImGuizmo::TRANSLATE;

    if (ImGui::IsKeyPressed(ImGuiKey_E, false))
        m_operation = ImGuizmo::ROTATE;

    if (ImGui::IsKeyPressed(ImGuiKey_R, false))
        m_operation = ImGuizmo::SCALE;
}

void TransformGizmo::render(const CameraViewport& viewport, omega::Entity selected_entity, const glm::mat4& view_matrix,
                            const glm::mat4& projection_matrix, omega::ComponentSystem& components,
                            omega::TransformSystem& transforms, omega::HierarchySystem& hierarchy)
{
    handle_shortcuts();

    if (selected_entity == omega::Entity{})
        return;

    if (!components.has<omega::Transform>(selected_entity))
        return;

    const ViewportRect& rect = viewport.rect();

    if (rect.size.x <= 0.0f || rect.size.y <= 0.0f)
        return;

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

    ImGuizmo::SetRect(rect.position.x, rect.position.y, rect.size.x, rect.size.y);

    // The gizmo manipulates world-space transforms
    omega::Matrix4 world_matrix = transforms.get_world_matrix(selected_entity);

    // Scaling is always local in ImGuizmo
    const ImGuizmo::MODE mode = m_operation == ImGuizmo::SCALE ? ImGuizmo::LOCAL : m_mode;

    omega::Matrix4 gizmo_projection = projection_matrix;

#if OMEGA_PROJECTION_FLIP_Y
    gizmo_projection[1][1] *= -1.0f;
#endif

    const bool changed = ImGuizmo::Manipulate(reinterpret_cast<const float*>(&view_matrix),
                                              reinterpret_cast<const float*>(&gizmo_projection),
                                              m_operation, mode, 
                                              reinterpret_cast<float*>(&world_matrix));

    if (!changed)
        return;

    // Convert the manipulated world matrix back to local space
    omega::Matrix4 local_matrix = world_matrix;
    const omega::Entity parent = hierarchy.get_parent(selected_entity);
    if (parent != omega::Entity{})
    {
        const omega::Matrix4 parent_world = transforms.get_world_matrix(parent);
        local_matrix = omega::inverse(parent_world) * world_matrix;
    }

    // Decompose the resulting local transformation
    omega::Vector3 scale;
    omega::Quaternion rotation;
    omega::Vector3 position;
    omega::Vector3 skew;
    omega::Vector4 perspective;
    if (!omega::decompose(local_matrix, scale, rotation, position, skew, perspective))
    {
        return;
    }

    auto transform = components.get<omega::Transform>(selected_entity);
    transform->position = position;
    transform->rotation = omega::normalize(rotation);
    transform->scale = scale;

    components.mark_dirty<omega::Transform>(selected_entity);
}

} // namespace omega_editor