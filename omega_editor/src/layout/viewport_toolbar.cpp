#include <omega_editor/layout/camera_viewport.hpp>
#include <omega_editor/layout/viewport_toolbar.hpp>
#include <omega_editor/icons/icons.hpp>
#include <omega_editor/gizmos/transform_gizmo.hpp>

#include <omega/core/component/component_system.hpp>
#include <omega/camera/camera.hpp>

#include <imgui.h>

#include <algorithm>
#include <string>

namespace
{

bool flat_picker_button(const char* id, const char* label)
{
    constexpr float padding_x = 8.0f;
    constexpr float chevron_width = 12.0f;
    constexpr float spacing = 5.0f;
    constexpr float height = 24.0f;

    const ImVec2 text_size = ImGui::CalcTextSize(label);

    const ImVec2 size{text_size.x + padding_x * 2.0f + spacing + chevron_width, height};

    ImGui::InvisibleButton(id, size);

    const bool hovered = ImGui::IsItemHovered();
    const bool clicked = ImGui::IsItemClicked();

    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();

    ImDrawList* draw = ImGui::GetWindowDrawList();

    if (hovered)
    {
        draw->AddRectFilled(min, max, IM_COL32(255, 255, 255, 16), 4.0f);
    }

    draw->AddText(ImVec2(min.x + padding_x, min.y + (height - text_size.y) * 0.5f),
                  hovered ? IM_COL32(230, 235, 245, 255) : IM_COL32(170, 180, 195, 255), label);

    omega_editor::icons::draw_chevron(draw, ImVec2(max.x - padding_x - chevron_width * 0.5f, min.y + height * 0.5f), true,
                                      IM_COL32(135, 145, 160, 220));

    return clicked;
}

void toolbar_separator()
{
    constexpr float toolbar_height = 30.0f;
    constexpr float separator_height = 16.0f;
    constexpr float separator_width = 1.0f;

    ImGui::SetCursorPosY((toolbar_height - separator_height) * 0.5f);

    const ImVec2 position = ImGui::GetCursorScreenPos();

    ImGui::Dummy(ImVec2(separator_width, separator_height));

    ImGui::GetWindowDrawList()->AddLine(position, ImVec2(position.x, position.y + separator_height),
                                        ImGui::GetColorU32(ImGuiCol_Separator));
}

using IconDrawFunction = void (*)(ImDrawList*, ImVec2, float, ImU32);

bool operation_button(const char* id, const char* tooltip, bool selected, IconDrawFunction draw_icon)
{
    constexpr float button_size = 24.0f;
    constexpr float icon_size = 16.0f;

    const float toolbar_height = 30.0f;

    ImGui::SetCursorPosY((toolbar_height - button_size) * 0.5f);

    const ImVec2 position = ImGui::GetCursorScreenPos();

    const bool clicked = ImGui::InvisibleButton(id, ImVec2(button_size, button_size));

    const bool hovered = ImGui::IsItemHovered();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    if (selected || hovered)
    {
        const ImU32 background = selected ? IM_COL32(65, 85, 110, 180) : ImGui::GetColorU32(ImGuiCol_ButtonHovered);

        draw_list->AddRectFilled(position, ImVec2(position.x + button_size, position.y + button_size), background,
                                 4.0f);
    }

    const ImU32 color = selected ? IM_COL32(190, 215, 245, 255) : ImGui::GetColorU32(hovered ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    const ImVec2 icon_position{position.x + (button_size - icon_size) * 0.5f,
                               position.y + (button_size - icon_size) * 0.5f};

    draw_icon(draw_list, icon_position, icon_size, color);

    if (hovered)
        ImGui::SetTooltip("%s", tooltip);

    return clicked;
}

bool toolbar_icon_button(const char* id, const char* tooltip, ImVec2 size,
                         const std::function<void(ImDrawList*, ImVec2, ImU32)>& draw_icon)
{
    const ImVec2 position = ImGui::GetCursorScreenPos();

    const bool clicked = ImGui::InvisibleButton(id, size);
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();

    ImDrawList* draw = ImGui::GetWindowDrawList();

    if (hovered || held)
    {
        const ImU32 background = ImGui::GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered);

        draw->AddRectFilled(position, {position.x + size.x, position.y + size.y}, background, 4.0f);
    }

    const ImU32 color = ImGui::GetColorU32(hovered || held ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    const ImVec2 center{position.x + size.x * 0.5f, position.y + size.y * 0.5f};

    draw_icon(draw, center, color);

    if (hovered && tooltip)
        ImGui::SetTooltip("%s", tooltip);

    return clicked;
}

} // namespace

namespace omega_editor
{

ViewportToolbar::ViewportToolbar(omega::ComponentSystem& components, const omega::Entity& editor_camera)
    : m_components(components), m_editor_camera(editor_camera)
{
}

ViewportToolbarAction ViewportToolbar::render(CameraViewport& viewport, TransformGizmo& gizmo)
{
    const auto& rect = viewport.rect();

    constexpr float toolbar_height = 30.0f;
    constexpr float tab_width = 36.0f;
    constexpr float tab_height = 12.0f;
    constexpr float button_height = 24.0f;

    const bool collapsed = viewport.fullscreen() && m_hidden;
    if (collapsed)
        return ViewportToolbarAction::None;

    ImGui::SetNextWindowPos(rect.position, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(rect.size.x, toolbar_height), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(collapsed ? 0.0f : 8.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 0.0f));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.12f, 0.15f, 0.90f));

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoScrollWithMouse;

    ViewportToolbarAction action = ViewportToolbarAction::None;
    bool camera_changed = false;

    if (ImGui::Begin("##CameraViewportToolbar", nullptr, flags))
    {
        ImDrawList* draw = ImGui::GetWindowDrawList();

        // Camera selection
        ImGui::SetCursorPosY((toolbar_height - button_height) * 0.5f);

        constexpr float icon_size = 16.0f;
        ImGui::SetCursorPosY((toolbar_height - icon_size) * 0.5f);
        const ImVec2 icon_position = ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2(icon_size, icon_size));
        icons::draw_camera(ImGui::GetWindowDrawList(), icon_position, icon_size,
                           ImGui::GetColorU32(ImGuiCol_TextDisabled));

        ImGui::SameLine(0.0f, 4.0f);

        draw_camera_picker(viewport, camera_changed);

        if (viewport.camera().type == omega::CameraType::Perspective)
        {
            // Aspect ratio selection
            ImGui::SameLine(0.0f, 6.0f);
            ImGui::SetCursorPosY((toolbar_height - button_height) * 0.5f);

            const std::string aspect_label = viewport.aspect_ratio_label();

            if (flat_picker_button("##AspectRatio", aspect_label.c_str()))
                ImGui::OpenPopup("##AspectRatioPresets");

            if (ImGui::BeginPopup("##AspectRatioPresets"))
            {
                const auto active_preset = viewport.aspect_ratio_preset();

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

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Camera viewport aspect ratio");
        }

        ImGui::SameLine(0.0f, 10.0f);
        toolbar_separator();
        ImGui::SameLine(0.0f, 10.0f);

        draw_transform_tools(gizmo);

        // Right-side controls
        const float controls_width = viewport.fullscreen() ? 94.0f : 70.0f;

        ImGui::SameLine();

        ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(),
                                      ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x - controls_width));

        ImGui::SetCursorPosY((toolbar_height - button_height) * 0.5f);

        // Fullscreen button
        constexpr float button_size = 24.0f;
        ImGui::SetCursorPosY((toolbar_height - button_size) * 0.5f);
        if (toolbar_icon_button("##Fullscreen", viewport.fullscreen() ? "Exit Fullscreen (F11)" : "Fullscreen (F11)",
                                ImVec2(button_size, button_size), [&](ImDrawList* draw, ImVec2 center, ImU32 color)
                                { icons::draw_fullscreen(draw, center, viewport.fullscreen(), color); }))
        {
            action = ViewportToolbarAction::ToggleFullscreen;
        }

        // Shortcut hint
        ImGui::SameLine(0.0f, 3.0f);

        ImGui::SetCursorPosY((toolbar_height - ImGui::GetTextLineHeight()) * 0.5f);

        ImGui::TextDisabled("F11");

        // Hide toolbar button, only in fullscreen
        if (viewport.fullscreen())
        {
            ImGui::SameLine(0.0f, 10.0f);
            ImGui::SetCursorPosY((toolbar_height - button_size) * 0.5f);

            if (toolbar_icon_button("##HideToolbar", "Hide Toolbar", ImVec2(button_size, button_size),
                                    [&](ImDrawList* draw, ImVec2 center, ImU32 color)
                                    { icons::draw_chevron(draw, center, false, color); }))
            {
                m_hidden = true;
            }
        }
    }

    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(4);

    if (camera_changed)
        action = ViewportToolbarAction::CameraChanged;

    return action;
}

void ViewportToolbar::show()
{
    m_hidden = false;
}

void ViewportToolbar::draw_camera_picker(CameraViewport& viewport, bool& changed)
{
    const CameraSelection active = viewport.camera();
    const CameraSelection editor_perspective{m_editor_camera, omega::CameraType::Perspective};
    const CameraSelection editor_orthographic{m_editor_camera, omega::CameraType::Orthographic};
    const bool editor_selected = active.entity == m_editor_camera;

    const auto editor_camera_label = [](const CameraSelection& selection)
    {
        const char* type = selection.type == omega::CameraType::Perspective ? "Perspective" : "Orthographic";

        return std::string("Editor Camera") + " - " + type;
    };

    const auto camera_label = [](const CameraSelection& selection)
    {
        const char* type = selection.type == omega::CameraType::Perspective ? "Perspective" : "Orthographic";

        return "Entity " + std::to_string(selection.entity.index) + " - " + type;
    };

    const std::string label = editor_selected ? editor_camera_label(active) : camera_label(active);

    ImGui::SetCursorPosY((30.0f - 24.0f) * 0.5f);

    if (flat_picker_button("##CameraPicker", label.c_str()))
        ImGui::OpenPopup("##CameraSelection");

    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Select viewport camera");

    if (!ImGui::BeginPopup("##CameraSelection"))
        return;

    const auto draw_editor_camera = [&](const char* name, CameraSelection selection)
    {
        const bool selected = viewport.camera() == selection;

        if (ImGui::Selectable(name, selected) && !selected)
        {
            viewport.set_camera(selection);
            changed = true;
        }

        if (selected)
            ImGui::SetItemDefaultFocus();
    };

    draw_editor_camera("Editor Camera - Perspective", editor_perspective);
    draw_editor_camera("Editor Camera - Orthographic", editor_orthographic);

    ImGui::Separator();

    const auto draw_cameras = [&](const auto& entities, omega::CameraType type)
    {
        for (omega::Entity entity : entities)
        {
            if (entity == m_editor_camera)
                continue;

            const CameraSelection selection{entity, type};
            const bool selected = active == selection;
            const std::string name = camera_label(selection);

            if (ImGui::Selectable(name.c_str(), selected) && !selected)
            {
                viewport.set_camera(selection);
                changed = true;
            }

            if (selected)
                ImGui::SetItemDefaultFocus();
        }
    };

    draw_cameras(m_components.entities_with<omega::PerspectiveCamera>(), omega::CameraType::Perspective);

    draw_cameras(m_components.entities_with<omega::OrthogonalCamera>(), omega::CameraType::Orthographic);

    ImGui::EndPopup();
}

void ViewportToolbar::draw_transform_tools(TransformGizmo& gizmo)
{
    const ImGuizmo::OPERATION active = gizmo.operation();

    if (operation_button("##Translate", "Translate (W)", active == ImGuizmo::TRANSLATE, icons::draw_translate))
    {
        gizmo.set_operation(ImGuizmo::TRANSLATE);
    }

    ImGui::SameLine(0.0f, 2.0f);

    if (operation_button("##Rotate", "Rotate (E)", active == ImGuizmo::ROTATE, icons::draw_rotate))
    {
        gizmo.set_operation(ImGuizmo::ROTATE);
    }

    ImGui::SameLine(0.0f, 2.0f);

    if (operation_button("##Scale", "Scale (R)", active == ImGuizmo::SCALE, icons::draw_scale))
    {
        gizmo.set_operation(ImGuizmo::SCALE);
    }
}

} // namespace omega_editor