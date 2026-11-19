#include <omega_editor/panels/inspection_panel.hpp>
#include <omega_editor/panels/component_editor_registry.hpp>

#include <omega/engine/engine.hpp>

#include <imgui.h>

namespace omega_editor
{

void InspectionPanel::render(omega::Engine& engine, omega::Entity& selected_entity, ComponentEditorRegistry& component_editor_registry)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    const float viewport_width = viewport->WorkSize.x;
    const float viewport_height = viewport->WorkSize.y;

    const float min_width = viewport_width * 0.15f;
    const float max_width = viewport_width * 0.35f;

    m_width_ratio = std::clamp(m_width_ratio, 0.15f, 0.35f);

    const float panel_width = viewport_width * m_width_ratio;

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport_width - panel_width, viewport->WorkPos.y));

    ImGui::SetNextWindowSize(ImVec2(panel_width, viewport_height));

    ImGui::SetNextWindowSizeConstraints(ImVec2(min_width, viewport_height), ImVec2(max_width, viewport_height));

    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    omega::ComponentSystem& components = engine.components();

    const bool valid_selection = selected_entity.is_valid() && engine.entities().is_alive(selected_entity);

    if (!valid_selection)
    {
        ImGui::TextDisabled("No entity selected");
    }
    else
    {
        ImGui::Text("Entity %u", selected_entity.index);
        ImGui::Separator();

        for (ComponentEditor& editor : component_editor_registry.editors())
        {
            if (!editor.has(selected_entity, components))
                continue;

            ImGui::PushID(static_cast<int>(editor.type.hash_code()));

            const float button_width = ImGui::GetFrameHeight();

            if (ImGui::BeginTable("ComponentHeader", 2))
            {
                ImGui::TableSetupColumn("Header", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Remove", ImGuiTableColumnFlags_WidthFixed, button_width);

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                const bool opened = ImGui::CollapsingHeader(editor.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

                ImGui::TableSetColumnIndex(1);
                if (ImGui::Button("X", ImVec2(button_width, button_width)))
                    m_pending_remove = &editor;

                ImGui::EndTable();

                if (opened)
                    editor.draw(selected_entity, components);
            }

            ImGui::PopID();
        }

        if (m_pending_remove.has_value())
        {
            m_pending_remove.value()->remove(selected_entity, components);
            m_pending_remove.reset();
        }
    }

    ImGui::Separator();

    if (ImGui::Button("Add Component", ImVec2(-1.0f, 0.0f)))
        ImGui::OpenPopup("AddComponentPopup");

    if (ImGui::BeginPopup("AddComponentPopup"))
    {
        for (ComponentEditor& editor : component_editor_registry.editors())
        {
            if (editor.has(selected_entity, components))
                continue;

            if (ImGui::MenuItem(editor.name.c_str()))
                m_pending_add = &editor;
        }

        ImGui::EndPopup();
    }

    if (m_pending_add.has_value())
    {
        m_pending_add.value()->create(selected_entity, components);
        m_pending_add.reset();
    }

    m_width_ratio = ImGui::GetWindowWidth() / viewport_width;

    ImGui::End();
}

} // namespace omega_editor