#ifndef OMEGA_EDITOR_PANEL_INSPECTION_HPP
#define OMEGA_EDITOR_PANEL_INSPECTION_HPP

#include <omega_editor/layout/panel_content.hpp>
#include <omega_editor/component_editor/component_editor.hpp>

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
class PanelSlot;

class InspectionPanel : public PanelContent
{
  public:
    InspectionPanel(omega::Engine& engine, omega::Entity& selected_entity, ComponentEditorRegistry& registry);

    const char* title() const override;
    void render() override;

  private:
    omega::Engine& m_engine;
    omega::Entity& m_selected_entity;
    ComponentEditorRegistry& m_registry;

    enum class ComponentAction
    {
        None,
        Add,
        Remove
    };

    ComponentAction m_pending_action = ComponentAction::None;
    std::optional<std::type_index> m_pending_type;
    bool m_confirmation_open = false;
    std::optional<std::type_index> m_navigation_target;

    void clear_pending_action();
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_PANEL_INSPECTION_HPP