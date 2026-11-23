#ifndef OMEGA_EDITOR_CAMERA_PANEL_HPP
#define OMEGA_EDITOR_CAMERA_PANEL_HPP

#include <omega/core/entity/entity.hpp>

#include <omega_editor/layout/panel_content.hpp>
#include <omega_editor/component_editor/component_editor_registry.hpp>

namespace omega
{

class Engine;

} // namespace omega

namespace omega_editor
{

class CameraViewport;
class EditorCameraControls;

class EditorCameraPanel : public PanelContent
{
  public:
    EditorCameraPanel(omega::Engine& engine, 
                      const omega::Entity& camera_entity, EditorCameraControls& controls,
                      ComponentEditorRegistry& registry);

    const char* title() const override;
    void render() override;

  private:
    void render_components();
    void render_navigation_settings();
    void render_navigation_bindings();

    omega::Engine& m_engine;
    const omega::Entity& m_camera_entity;
    EditorCameraControls& m_controls;
    ComponentEditorRegistry& m_registry;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_CAMERA_PANEL_HPP