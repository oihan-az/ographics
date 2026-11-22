#ifndef OMEGA_EDITOR_CAMERA_PANEL_HPP
#define OMEGA_EDITOR_CAMERA_PANEL_HPP

#include <omega_editor/layout/panel_content.hpp>
#include <omega_editor/component_editor/component_editor_registry.hpp>

namespace omega
{

class Engine;
class Entity;

} // namespace omega

namespace omega_editor
{

class EditorCameraPanel : public PanelContent
{
  public:
    EditorCameraPanel(omega::Engine& engine, omega::Entity& camera_entity, ComponentEditorRegistry& registry);

    const char* title() const override;
    void render() override;

  private:
    omega::Engine& m_engine;
    omega::Entity& m_camera_entity;
    ComponentEditorRegistry& m_registry;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_CAMERA_PANEL_HPP