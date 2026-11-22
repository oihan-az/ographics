#include <omega_editor/layout/camera_viewport.hpp>
#include <omega_editor/layout/viewport_toolbar.hpp>
#include <omega_editor/icons/icons.hpp>

#include <imgui.h>

#include <algorithm>

namespace omega_editor
{

ViewportToolbarAction ViewportToolbar::render(CameraViewport& viewport)
{
    const auto& rect = viewport.rect();

    if (rect.size.x < 120.0f || rect.size.y < 30.0f)
        return ViewportToolbarAction::None;

    constexpr float toolbar_height = 30.0f;
    constexpr float tab_width = 36.0f;
    constexpr float tab_height = 12.0f;

    const bool collapsed = viewport.fullscreen() && m_hidden;

    if (collapsed)
    {
        ImGui::SetNextWindowPos(ImVec2(rect.position.x + (rect.size.x - tab_width) * 0.5f, rect.position.y),
                                ImGuiCond_Always);

        ImGui::SetNextWindowSize(ImVec2(tab_width, tab_height), ImGuiCond_Always);
    }
    else
    {
        ImGui::SetNextWindowPos(rect.position, ImGuiCond_Always);

        ImGui::SetNextWindowSize(ImVec2(rect.size.x, toolbar_height), ImGuiCond_Always);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(collapsed ? 0.0f : 8.0f, 0.0f));

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 0.0f));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.12f, 0.15f, 0.90f));

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoScrollWithMouse;

    ViewportToolbarAction action = ViewportToolbarAction::None;

    if (!collapsed)
    {
        if (ImGui::Begin("##CameraViewportToolbar", nullptr, flags))
        {
            ImDrawList* draw = ImGui::GetWindowDrawList();

            // Viewport information
            ImGui::SetCursorPosY((toolbar_height - ImGui::GetTextLineHeight()) * 0.5f);

            ImGui::SetNextItemWidth(75.0f);

            // Aspect ratio picker
            {
                const char* label = viewport.aspect_ratio_label();
                const auto active_preset = viewport.aspect_ratio_preset();

                constexpr float padding_x = 8.0f;
                constexpr float chevron_width = 12.0f;
                constexpr float spacing = 5.0f;

                const ImVec2 text_size = ImGui::CalcTextSize(label);

                const ImVec2 button_size{text_size.x + padding_x * 2.0f + spacing + chevron_width, 24.0f};

                ImGui::SetCursorPosY((toolbar_height - button_size.y) * 0.5f);

                ImGui::InvisibleButton("##AspectRatio", button_size);

                const bool hovered = ImGui::IsItemHovered();
                const bool clicked = ImGui::IsItemClicked();

                const ImVec2 min = ImGui::GetItemRectMin();
                const ImVec2 max = ImGui::GetItemRectMax();

                ImDrawList* draw = ImGui::GetWindowDrawList();

                if (hovered)
                {
                    draw->AddRectFilled(min, max, IM_COL32(255, 255, 255, 16), 4.0f);
                }

                draw->AddText(ImVec2(min.x + padding_x, min.y + (button_size.y - text_size.y) * 0.5f),
                              hovered ? IM_COL32(230, 235, 245, 255) : IM_COL32(170, 180, 195, 255), label);

                icons::draw_chevron(draw,
                                    ImVec2(max.x - padding_x - chevron_width * 0.5f, min.y + button_size.y * 0.5f),
                                    true, IM_COL32(135, 145, 160, 220));

                if (clicked)
                    ImGui::OpenPopup("##AspectRatioPresets");

                if (ImGui::BeginPopup("##AspectRatioPresets"))
                {
                    for (const auto& info : aspect_ratio_presets)
                    {
                        const bool selected = active_preset && *active_preset == info.preset;

                        if (ImGui::Selectable(info.label, selected))
                            viewport.set_aspect_ratio(info.preset);

                        if (selected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndPopup();
                }
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Camera viewport aspect ratio");

            // Right-side controls
            const float controls_width = viewport.fullscreen() ? 94.0f : 70.0f;

            ImGui::SameLine();

            ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x - controls_width));

            ImGui::SetCursorPosY(3.0f);

            // Fullscreen button
            ImGui::PushID("Fullscreen");

            if (ImGui::InvisibleButton("##Fullscreen", ImVec2(22.0f, 24.0f)))
            {
                action = ViewportToolbarAction::ToggleFullscreen;
            }

            const ImVec2 min = ImGui::GetItemRectMin();
            const ImVec2 max = ImGui::GetItemRectMax();

            icons::draw_fullscreen(draw, {(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f}, viewport.fullscreen(),
                                   IM_COL32(225, 230, 240, 240));

            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip(viewport.fullscreen() ? "Exit fullscreen (F11)" : "Enter fullscreen (F11)");
            }

            ImGui::PopID();

            // Shortcut hint
            ImGui::SameLine(0.0f, 3.0f);

            ImGui::SetCursorPosY((toolbar_height - ImGui::GetTextLineHeight()) * 0.5f);

            ImGui::TextDisabled("F11");

            // Hide toolbar button
            if (viewport.fullscreen())
            {
                ImGui::SameLine(0.0f, 10.0f);
                ImGui::SetCursorPosY(3.0f);

                if (ImGui::InvisibleButton("##HideToolbar", ImVec2(22.0f, 24.0f)))
                {
                    m_hidden = true;
                }

                const ImVec2 hide_min = ImGui::GetItemRectMin();
                const ImVec2 hide_max = ImGui::GetItemRectMax();

                icons::draw_chevron(draw, {(hide_min.x + hide_max.x) * 0.5f, (hide_min.y + hide_max.y) * 0.5f}, false,
                                    IM_COL32(150, 160, 175, 220));

                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Hide toolbar");
            }
        }

        ImGui::End();
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(4);

    return action;
}

void ViewportToolbar::show()
{
    m_hidden = false;
}

} // namespace omega_editor