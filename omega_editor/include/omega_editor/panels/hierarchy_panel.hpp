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

class HierarchyPanel
{
  public:
    void render(omega::Engine& engine);

  private:
    float m_width_ratio = 0.20f;
    std::optional<omega::Entity> m_pending_delete;

    void render_entity(omega::Entity entity, omega::HierarchySystem& hierarchy);
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_PANEL_HIERARCHY_HPP