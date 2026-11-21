#ifndef OMEGA_EDITOR_HPP
#define OMEGA_EDITOR_HPP

#include <omega_editor/layout/panel_slot.hpp>
#include <omega_editor/layout/camera_viewport.hpp>
#include <omega_editor/layout/viewport_toolbar.hpp>
#include <omega_editor/panels/hierarchy_panel.hpp>
#include <omega_editor/panels/inspection_panel.hpp>
#include <omega_editor/panels/component_editor_registry.hpp>
#include <omega_editor/gizmos/transform_gizmo.hpp>

namespace omega
{

class Engine;

} // namespace omega

namespace omega_editor
{

class Editor
{
  public:
    Editor(omega::Engine& engine);

    void initialize();
    void render();

    omega::Entity editor_camera() const;
    const CameraViewport& camera_viewport() const;

  private:
    void create_editor_camera();
    omega::Entity m_camera_entity{};

    omega::Engine& m_engine;

    omega::Entity m_selected_entity;

    void register_component_editors();
    ComponentEditorRegistry m_component_editors;

    HierarchyPanel m_hierarchy_panel;
    InspectionPanel m_inspection_panel;

    PanelSlot m_left_slot{PanelAnchor::Left, {{0.20f, 0.15f, 0.35f}, {}}};
    PanelSlot m_right_slot{PanelAnchor::Right, {{0.20f, 0.15f, 0.35f}, {}}};

    CameraViewport m_camera_viewport;
    ViewportToolbar m_viewport_toolbar;
    void update_camera_viewport();

    TransformGizmo m_transform_gizmo;
    void render_transform_gizmo();
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_HPP