#ifndef OMEGA_EDITOR_PANEL_INSPECTION_HPP
#define OMEGA_EDITOR_PANEL_INSPECTION_HPP

#include <omega_editor/panels/component_editor.hpp>

#include <optional>
#include <typeindex>

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
    enum class ComponentAction
    {
        None,
        Add,
        Remove
    };

    float m_width_ratio = 0.20f;

    ComponentAction m_pending_action = ComponentAction::None;
    std::optional<std::type_index> m_pending_type;
    bool m_confirmation_open = false;
    std::optional<std::type_index> m_navigation_target;

    void clear_pending_action();
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_PANEL_INSPECTION_HPP