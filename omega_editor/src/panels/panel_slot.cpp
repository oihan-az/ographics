#include <omega_editor/layout/panel_slot.hpp>

#include <imgui.h>

#include <algorithm>

namespace omega_editor
{

PanelSlot::PanelSlot(PanelAnchor anchor, float width_ratio, float min_width_ratio, float max_width_ratio)
    : m_anchor(anchor), m_width_ratio(width_ratio), m_min_width_ratio(min_width_ratio),
      m_max_width_ratio(max_width_ratio)
{
}

bool PanelSlot::begin(const char* title)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    const float viewport_width = viewport->WorkSize.x;
    const float viewport_height = viewport->WorkSize.y;

    m_width_ratio = std::clamp(m_width_ratio, m_min_width_ratio, m_max_width_ratio);

    const float panel_width = viewport_width * m_width_ratio;

    float x = viewport->WorkPos.x;

    if (m_anchor == PanelAnchor::Right)
        x += viewport_width - panel_width;

    ImGui::SetNextWindowPos(ImVec2(x, viewport->WorkPos.y));

    ImGui::SetNextWindowSize(ImVec2(panel_width, viewport_height));

    ImGui::SetNextWindowSizeConstraints(ImVec2(viewport_width * m_min_width_ratio, viewport_height),
                                        ImVec2(viewport_width * m_max_width_ratio, viewport_height));

    return ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
}

void PanelSlot::end()
{
    const float viewport_width = ImGui::GetMainViewport()->WorkSize.x;

    if (viewport_width > 0.0f)
    {
        m_width_ratio = std::clamp(ImGui::GetWindowWidth() / viewport_width, m_min_width_ratio, m_max_width_ratio);
    }

    ImGui::End();
}

float PanelSlot::width_ratio() const
{
    return m_width_ratio;
}

} // namespace omega_editor