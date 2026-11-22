#ifndef OMEGA_EDITOR_HPP
#define OMEGA_EDITOR_HPP

#include <omega_editor/layout/panel_slot.hpp>
#include <omega_editor/layout/camera_viewport.hpp>
#include <omega_editor/layout/viewport_toolbar.hpp>
#include <omega_editor/layout/panel.hpp>
#include <omega_editor/layout/tabbed_panel.hpp>
#include <omega_editor/panels/hierarchy_panel.hpp>
#include <omega_editor/panels/inspection_panel.hpp>
#include <omega_editor/panels/editor_camera_panel.hpp>
#include <omega_editor/panels/performance_panel.hpp>
#include <omega_editor/component_editor/component_editor_registry.hpp>
#include <omega_editor/gizmos/transform_gizmo.hpp>
#include <omega_editor/camera/editor_camera_controls.hpp>
#include <omega_editor/camera/editor_camera_controller.hpp>

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
    bool camera_changed() const;

  private:
    omega::Engine& m_engine;
    omega::Entity m_selected_entity;

    void create_editor_camera();
    omega::Entity m_camera_entity;
    bool m_camera_changed = false;
    bool validate_camera_selection();

    void register_component_editors();
    ComponentEditorRegistry m_component_editors;

    PanelSlot m_left_slot{PanelAnchor::Left, {{0.20f, 0.15f, 0.35f}, {}}};
    PanelSlot m_right_slot{PanelAnchor::Right, {{0.20f, 0.15f, 0.35f}, {}}};

    Panel m_hierarchy_panel{m_left_slot, "Hierarchy"};
    TabbedPanel m_properties_panel{m_right_slot, "Properties"};
 
    CameraViewport m_camera_viewport;

    EditorCameraControls m_editor_camera_controls;
    EditorCameraController m_editor_camera_controller;
    void update_editor_camera();

    HierarchyPanel m_hierarchy_content{m_engine, m_selected_entity, m_camera_entity};
    InspectionPanel m_inspection_content{m_engine, m_selected_entity, m_component_editors};
    EditorCameraPanel m_editor_camera_content{m_engine, m_camera_entity, m_editor_camera_controls, m_component_editors};
    PerformancePanel m_performance_content{m_engine};

    ViewportToolbar m_viewport_toolbar;
    void update_camera_viewport();

    TransformGizmo m_transform_gizmo;
    void render_transform_gizmo();
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_HPP