#ifndef OMEGA_EDITOR_PANEL_HIERARCHY_HPP
#define OMEGA_EDITOR_PANEL_HIERARCHY_HPP

#include <omega/core/entity/entity.hpp>

#include <optional>

namespace omega
{

class Engine;
struct HierarchySystem;

} // namespace omega

namespace omega_editor
{

class PanelSlot;

class HierarchyPanel
{
  public:
    void render(PanelSlot& slot, omega::Engine& engine, omega::Entity& selected_entity);

  private:
    std::optional<omega::Entity> m_pending_delete;

    void render_entity(omega::Entity entity, omega::HierarchySystem& hierarchy, omega::Entity& selected_entity);
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_PANEL_HIERARCHY_HPP