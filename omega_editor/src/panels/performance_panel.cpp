#include <omega_editor/panels/performance_panel.hpp>

#include <omega/core/performance/performance_system.hpp>
#include <omega/engine/engine.hpp>

#include <imgui.h>

namespace omega_editor
{

PerformancePanel::PerformancePanel(omega::Engine& engine) : m_engine(engine) {}

const char* PerformancePanel::title() const
{
    return "Performance";
}

void PerformancePanel::render()
{
    render_frame_metrics();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    render_cpu_metrics();
}

void PerformancePanel::render_frame_metrics()
{
    const auto& metrics = m_engine.performance().metrics();

    if (ImGui::CollapsingHeader("Frame", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::BeginTable("##FrameMetrics", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg))
        {
            const auto row = [](const char* name, double value, const char* unit)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(name);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.2f %s", value, unit);
            };

            row("FPS", metrics.frame.fps, "");
            row("Frame Time", metrics.cpu.frame_ms, "ms");
            row("Average", metrics.frame.average_ms, "ms");
            row("Minimum", metrics.frame.minimum_ms, "ms");
            row("Maximum", metrics.frame.maximum_ms, "ms");

            ImGui::EndTable();
        }
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Frame Time History");

    render_frame_history();
}

void PerformancePanel::render_cpu_metrics()
{
    const auto& metrics = m_engine.performance().metrics();

    if (!ImGui::CollapsingHeader("CPU", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    if (!ImGui::BeginTable("##CpuMetrics", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg))
    {
        return;
    }

    ImGui::TableSetupColumn("Stage");
    ImGui::TableSetupColumn("Time");
    ImGui::TableHeadersRow();

    for (const auto& metric : metrics.cpu.stages)
    {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(metric.name.c_str());

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.3f ms", metric.elapsed_ms);
    }

    ImGui::EndTable();
}

void PerformancePanel::render_frame_history()
{
    omega::PerformanceSystem& performance = m_engine.performance();
    const std::size_t count = performance.frame_history_size();

    if (count == 0)
        return;

    float maximum = 0.0f;

    for (std::size_t i = 0; i < count; ++i)
    {
        const float value = static_cast<float>(performance.frame_history_at(i));

        m_frame_history[i] = value;
        maximum = std::max(maximum, value);
    }

    // Give the graph some breathing room above its highest sample
    const float graph_max = std::max(16.67f, maximum * 1.2f);

    char overlay[64];

    std::snprintf(overlay, sizeof(overlay), "Avg %.2f ms", performance.metrics().frame.average_ms);

    ImGui::PlotLines("##FrameHistory", m_frame_history.data(), static_cast<int>(count), 0, overlay, 0.0f, graph_max,
                     ImVec2(-1.0f, 100.0f));

    ImGui::TextDisabled("%zu frames | 0 - %.1f ms", count, graph_max);
}

} // namespace omega_editor