#include <omega_editor/layout/panel_slot.hpp>

#include <imgui.h>

#include <algorithm>

namespace
{

bool uses_width(omega_editor::PanelAnchor anchor)
{
    using omega_editor::PanelAnchor;

    return anchor != PanelAnchor::Top && anchor != PanelAnchor::Bottom;
}

bool uses_height(omega_editor::PanelAnchor anchor)
{
    using omega_editor::PanelAnchor;

    return anchor != PanelAnchor::Left && anchor != PanelAnchor::Right;
}

ImVec2 anchor_pivot(omega_editor::PanelAnchor anchor)
{
    using omega_editor::PanelAnchor;

    switch (anchor)
    {
    case PanelAnchor::Left:
    case PanelAnchor::Top:
    case PanelAnchor::TopLeft:
        return ImVec2(0.0f, 0.0f);

    case PanelAnchor::Right:
    case PanelAnchor::TopRight:
        return ImVec2(1.0f, 0.0f);

    case PanelAnchor::Bottom:
    case PanelAnchor::BottomLeft:
        return ImVec2(0.0f, 1.0f);

    case PanelAnchor::BottomRight:
        return ImVec2(1.0f, 1.0f);
    }

    return ImVec2(0.0f, 0.0f);
}

float clamp_ratio(float value, const omega_editor::PanelDimension& dimension)
{
    const float min = std::clamp(dimension.min, 0.0f, 1.0f);
    const float max = std::clamp(dimension.max, min, 1.0f);

    return std::clamp(value, min, max);
}

} // namespace

namespace omega_editor
{

PanelSlot::PanelSlot(PanelAnchor anchor, PanelSize size) : m_anchor(anchor), m_size(size) {}

bool PanelSlot::begin(const char* title)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    const ImVec2 viewport_pos = viewport->WorkPos;
    const ImVec2 viewport_size = viewport->WorkSize;

    const ImVec2 pivot = anchor_pivot(m_anchor);

    // Clamp the dimensions used by this anchor
    if (uses_width(m_anchor))
    {
        m_size.width.size = clamp_ratio(m_size.width.size, m_size.width);
    }

    if (uses_height(m_anchor))
    {
        m_size.height.size = clamp_ratio(m_size.height.size, m_size.height);
    }

    // Side slots occupy the full height
    // Top/bottom slots occupy the full width
    const float width_ratio = uses_width(m_anchor) ? m_size.width.size : 1.0f;
    const float height_ratio = uses_height(m_anchor) ? m_size.height.size : 1.0f;
    const ImVec2 window_size(viewport_size.x * width_ratio, viewport_size.y * height_ratio);

    // Pivot determines which edges remain anchored
    const ImVec2 window_pos(viewport_pos.x + viewport_size.x * pivot.x, viewport_pos.y + viewport_size.y * pivot.y);

    // Calculate resizing constraints
    const float min_width = uses_width(m_anchor) ? viewport_size.x * std::clamp(m_size.width.min, 0.0f, 1.0f) : viewport_size.x;
    const float max_width = uses_width(m_anchor) ? viewport_size.x * std::clamp(m_size.width.max, m_size.width.min, 1.0f) : viewport_size.x;
    const float min_height = uses_height(m_anchor) ? viewport_size.y * std::clamp(m_size.height.min, 0.0f, 1.0f) : viewport_size.y;
    const float max_height = uses_height(m_anchor) ? viewport_size.y * std::clamp(m_size.height.max, m_size.height.min, 1.0f)
                                                   : viewport_size.y;

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, pivot);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
    ImGui::SetNextWindowSizeConstraints(ImVec2(min_width, min_height), ImVec2(max_width, max_height));

    return ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
}

void PanelSlot::end()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    const ImVec2 window_size = ImGui::GetWindowSize();
    const ImVec2 viewport_size = viewport->WorkSize;

    // Remember user resizing
    if (uses_width(m_anchor) && viewport_size.x > 0.0f)
    {
        m_size.width.size = clamp_ratio(window_size.x / viewport_size.x, m_size.width);
    }

    if (uses_height(m_anchor) && viewport_size.y > 0.0f)
    {
        m_size.height.size = clamp_ratio(window_size.y / viewport_size.y, m_size.height);
    }

    const ImVec2 position = ImGui::GetWindowPos();
    const ImVec2 size = ImGui::GetWindowSize();
    m_bounds = {position.x, position.y, size.x, size.y};

    ImGui::End();
}

PanelAnchor PanelSlot::anchor() const
{
    return m_anchor;
}

float PanelSlot::width_ratio() const
{
    return uses_width(m_anchor) ? m_size.width.size : 1.0f;
}

float PanelSlot::height_ratio() const
{
    return uses_height(m_anchor) ? m_size.height.size : 1.0f;
}

const PanelBounds& PanelSlot::bounds() const
{
    return m_bounds;
}

} // namespace omega_editor