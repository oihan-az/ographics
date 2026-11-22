#include <omega_editor/panels/performance_panel.hpp>

#include <imgui.h>

#include <algorithm>
#include <cstdio>

namespace omega_editor
{

PerformancePanel::PerformancePanel(const omega::PerformanceSystem& performance)
    : m_performance(performance)
{
}

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
    if (!ImGui::CollapsingHeader("Frame", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    const auto& metrics = m_performance.metrics();

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

    ImGui::Spacing();
    ImGui::TextDisabled("Frame Time History");

    render_frame_history();
}

void PerformancePanel::render_frame_history()
{
    const std::size_t count = m_performance.frame_history_size();

    if (count == 0)
        return;

    float maximum = 0.0f;

    for (std::size_t i = 0; i < count; ++i)
    {
        const float value = static_cast<float>(m_performance.frame_history_at(i));

        m_frame_history[i] = value;
        maximum = std::max(maximum, value);
    }

    const float graph_max = std::max(16.67f, maximum * 1.2f);

    char overlay[64];

    std::snprintf(overlay, sizeof(overlay), "Avg %.2f ms", m_performance.metrics().frame.average_ms);

    ImGui::PlotLines("##FrameHistory", m_frame_history.data(), static_cast<int>(count), 0, overlay, 0.0f, graph_max,
                     ImVec2(-1.0f, 100.0f));

    ImGui::TextDisabled("%zu frames | 0 - %.1f ms", count, graph_max);
}

void PerformancePanel::render_cpu_metrics()
{
    if (!ImGui::CollapsingHeader("CPU", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    const auto& scopes = m_performance.metrics().cpu.scopes;

    if (scopes.empty())
    {
        ImGui::TextDisabled("No CPU profiling data");
        return;
    }

    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp;

    if (!ImGui::BeginTable("##CpuProfiler", 3, flags))
        return;

    ImGui::TableSetupColumn("Scope", ImGuiTableColumnFlags_WidthStretch, 1.0f);

    ImGui::TableSetupColumn("Total", ImGuiTableColumnFlags_WidthFixed, 85.0f);

    ImGui::TableSetupColumn("Self", ImGuiTableColumnFlags_WidthFixed, 85.0f);

    ImGui::TableHeadersRow();

    for (std::size_t i = 0; i < scopes.size(); ++i)
    {
        if (scopes[i].parent != omega::invalid_cpu_scope)
            continue;

        render_cpu_scope(static_cast<omega::CpuScopeId>(i), scopes);
    }

    ImGui::EndTable();
}

void PerformancePanel::render_cpu_scope(omega::CpuScopeId id, const std::vector<omega::CpuMetric>& scopes)
{
    const auto& scope = scopes[id];

    bool has_children = false;

    for (const auto& child : scopes)
    {
        if (child.parent == id)
        {
            has_children = true;
            break;
        }
    }

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);

    ImGui::PushID(static_cast<int>(id));

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;

    if (!has_children)
    {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    const bool opened = ImGui::TreeNodeEx("##Scope", flags, "%.*s", static_cast<int>(scope.name.size()), scope.name.data());

    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%.3f ms", scope.elapsed_ms);

    ImGui::TableSetColumnIndex(2);
    ImGui::Text("%.3f ms", scope.self_ms);

    if (opened && has_children)
    {
        for (std::size_t i = 0; i < scopes.size(); ++i)
        {
            if (scopes[i].parent != id)
                continue;

            render_cpu_scope(static_cast<omega::CpuScopeId>(i), scopes);
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}

} // namespace omega_editor