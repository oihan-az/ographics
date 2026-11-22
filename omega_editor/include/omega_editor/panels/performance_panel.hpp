#ifndef OMEGA_EDITOR_PERFORMANCE_PANEL_HPP
#define OMEGA_EDITOR_PERFORMANCE_PANEL_HPP

#include <omega_editor/layout/panel_content.hpp>
#include <omega/core//performance/performance_system.hpp>

namespace omega
{
class Engine;
} // namespace omega

namespace omega_editor
{

class PerformancePanel : public PanelContent
{
  public:
    PerformancePanel(omega::Engine& engine);

    const char* title() const override;
    void render() override;

  private:
    void render_frame_metrics();
    void render_cpu_metrics();

    void render_frame_history();
    std::array<float, omega::PerformanceSystem::history_capacity> m_frame_history{};

    omega::Engine& m_engine;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_PERFORMANCE_PANEL_HPP