#include <omega_editor/panels/editor_camera_panel.hpp>
#include <omega_editor/component_editor/component_editor.hpp>
#include <omega_editor/layout/camera_viewport.hpp>
#include <omega_editor/camera/editor_camera_controls.hpp>

#include <omega/core/math/angle.hpp>
#include <omega/core/math/quaternion.hpp>
#include <omega/core/component/component_system.hpp>

#include <omega/engine/engine.hpp>
#include <omega/camera/camera.hpp>
#include <omega/transform/transform.hpp>

#include <imgui.h>

namespace omega_editor
{

EditorCameraPanel::EditorCameraPanel(omega::Engine& engine, 
                                     const omega::Entity& camera_entity, EditorCameraControls& controls,
                                     ComponentEditorRegistry& registry)
    : m_engine(engine), m_camera_entity(camera_entity), m_controls(controls), m_registry(registry)
{
}

const char* EditorCameraPanel::title() const
{
    return "Editor Camera";
}

void EditorCameraPanel::render()
{
    ImGui::Separator();

    render_navigation_settings();
    render_navigation_bindings();

    ImGui::Separator();

    render_components();
}

void EditorCameraPanel::render_components()
{
    auto& components = m_engine.components();

    for (const auto& editor : m_registry.editors())
    {
        if (!editor.has(m_camera_entity, components))
            continue;

        ImGui::PushID(editor.name.c_str());

        if (ImGui::CollapsingHeader(editor.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            editor.draw(m_camera_entity, components);
        }

        ImGui::PopID();
    }
}

void EditorCameraPanel::render_navigation_settings()
{
    if (!ImGui::CollapsingHeader("Navigation", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    ImGui::PushID("NavigationSettings");

    ImGui::TextDisabled("Movement");

    ImGui::DragFloat("Movement Speed", &m_controls.movement_speed, 0.1f, 0.1f, 100.0f, "%.1f");

    ImGui::DragFloat("Boost Multiplier", &m_controls.boost_multiplier, 0.1f, 1.0f, 10.0f, "%.1fx");

    ImGui::Spacing();
    ImGui::TextDisabled("Rotation");

    ImGui::DragFloat("Look Sensitivity", &m_controls.look_sensitivity, 0.01f, 0.01f, 2.0f, "%.2f");

    ImGui::DragFloat("Orbit Sensitivity", &m_controls.orbit_sensitivity, 0.01f, 0.01f, 2.0f, "%.2f");

    ImGui::Spacing();
    ImGui::TextDisabled("Translation & Zoom");

    ImGui::DragFloat("Pan Sensitivity", &m_controls.pan_sensitivity, 0.01f, 0.01f, 10.0f, "%.2f");

    ImGui::DragFloat("Dolly Sensitivity", &m_controls.dolly_sensitivity, 0.01f, 0.01f, 10.0f, "%.2f");

    ImGui::PopID();
}

void EditorCameraPanel::render_navigation_bindings()
{
    if (!ImGui::CollapsingHeader("Controls"))
        return;

    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerH;

    if (!ImGui::BeginTable("##CameraControls", 2, flags))
        return;

    ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthStretch, 1.0f);
    ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch, 1.2f);
    ImGui::TableHeadersRow();

    const auto row = [](const char* action, const char* input)
    {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(action);

        ImGui::TableSetColumnIndex(1);
        ImGui::TextDisabled("%s", input);
    };

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextDisabled("FREE-FLY");

    row("Look Around", "RMB + Mouse");
    row("Move", "RMB + W / A / S / D");
    row("Move Up / Down", "RMB + E / Q");
    row("Move Faster", "RMB + Shift");
    row("Pan", "MMB + Mouse");
    row("Dolly", "Mouse Wheel");
    row("Adjust Speed", "RMB + Wheel");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextDisabled("ORBIT");

    row("Orbit", "Alt + LMB");
    row("Pan", "Alt + MMB");
    row("Dolly", "Alt + RMB");
    row("Zoom", "Alt + Wheel");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextDisabled("SELECTION");

    row("Focus Selection", "F");

    ImGui::EndTable();
}

} // namespace omega_editor