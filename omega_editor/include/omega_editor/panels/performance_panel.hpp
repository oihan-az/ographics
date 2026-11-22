#ifndef OMEGA_EDITOR_PERFORMANCE_PANEL_CONTENT_HPP
#define OMEGA_EDITOR_PERFORMANCE_PANEL_CONTENT_HPP

#include <omega_editor/layout/panel_content.hpp>

#include <omega/core/performance/performance_system.hpp>

#include <array>

namespace omega_editor
{

class PerformancePanel : public PanelContent
{
  public:
    explicit PerformancePanel(const omega::PerformanceSystem& performance);

    const char* title() const override;
    void render() override;

  private:
    void render_frame_metrics();
    void render_frame_history();
    void render_cpu_metrics();

    void render_cpu_scope(omega::CpuScopeId id, const std::vector<omega::CpuMetric>& scopes);

    const omega::PerformanceSystem& m_performance;

    std::array<float, omega::PerformanceSystem::history_capacity> m_frame_history{};
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_PERFORMANCE_PANEL_CONTENT_HPP