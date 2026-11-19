#ifndef OMEGA_EDITOR_PANEL_INSPECTION_HPP
#define OMEGA_EDITOR_PANEL_INSPECTION_HPP

#include <omega_editor/panels/component_editor.hpp>

#include <optional>

namespace omega
{

class Engine;
class Entity;

} // namespace omega

namespace omega_editor
{

class ComponentEditorRegistry;

class InspectionPanel
{
  public:
    void render(omega::Engine& engine, omega::Entity& selected_entity, ComponentEditorRegistry& component_editor_registry);

  private:
    float m_width_ratio = 0.20f;

    std::optional<ComponentEditor*> m_pending_remove;
    std::optional<ComponentEditor*> m_pending_add;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_PANEL_INSPECTION_HPP