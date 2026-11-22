#ifndef OMEGA_EDITOR_PANEL_HIERARCHY_HPP
#define OMEGA_EDITOR_PANEL_HIERARCHY_HPP

#include <omega/core/entity/entity.hpp>
#include <omega_editor/layout/panel_content.hpp>

#include <optional>

namespace omega
{

class Engine;
struct HierarchySystem;

} // namespace omega

namespace omega_editor
{

class PanelSlot;

class HierarchyPanel : public PanelContent
{
  public:
    HierarchyPanel(omega::Engine& engine, omega::Entity& selected_entity, omega::Entity& excluded_entity);

    const char* title() const override;
    void render() override;

  private:
    omega::Engine& m_engine;
    omega::Entity& m_selected_entity;
    omega::Entity& m_excluded_entity;

    std::optional<omega::Entity> m_pending_delete;

    void render_entity(omega::Entity entity, omega::HierarchySystem& hierarchy, omega::Entity& selected_entity);
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_PANEL_HIERARCHY_HPP