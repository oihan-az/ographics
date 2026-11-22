#ifndef OMEGA_EDITOR_TRANSFORM_GIZMO_HPP
#define OMEGA_EDITOR_TRANSFORM_GIZMO_HPP

#include <omega_editor/layout/camera_viewport.hpp>

#include <omega/core/entity/entity.hpp>

#include <ImGuizmo.h>
#include <glm/mat4x4.hpp>

namespace omega
{

class ComponentSystem;
class TransformSystem;
class HierarchySystem;

} // namespace omega

namespace omega_editor
{

class TransformGizmo
{
  public:
    void render(const CameraViewport& viewport, omega::Entity selected_entity, const glm::mat4& view_matrix,
                const glm::mat4& projection_matrix, omega::ComponentSystem& components,
                omega::TransformSystem& transforms, omega::HierarchySystem& hierarchy, 
                bool navigation_active);

  private:
    void handle_shortcuts();

    ImGuizmo::OPERATION m_operation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE m_mode = ImGuizmo::LOCAL;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_TRANSFORM_GIZMO_HPP