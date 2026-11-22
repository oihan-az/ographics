#include <omega_editor/panels/editor_camera_panel.hpp>
#include <omega_editor/component_editor/component_editor.hpp>
#include <omega_editor/layout/camera_viewport.hpp>

#include <omega/core/math/angle.hpp>
#include <omega/core/math/quaternion.hpp>
#include <omega/core/component/component_system.hpp>

#include <omega/engine/engine.hpp>
#include <omega/camera/camera.hpp>
#include <omega/transform/transform.hpp>

#include <imgui.h>

namespace omega_editor
{

EditorCameraPanel::EditorCameraPanel(omega::Engine& engine, const omega::Entity& camera_entity,
                                                   ComponentEditorRegistry& registry)
    : m_engine(engine), m_camera_entity(camera_entity), m_registry(registry)
{
}

const char* EditorCameraPanel::title() const
{
    return "Editor Camera";
}

void EditorCameraPanel::render()
{
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

} // namespace omega_editor