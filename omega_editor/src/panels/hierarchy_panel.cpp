#include <omega_editor/panels/hierarchy_panel.hpp>
#include <omega_editor/layout/panel_slot.hpp>

#include <omega/core/entity/entity_system.hpp>
#include <omega/core/hierarchy/hierarchy_system.hpp>
#include <omega/engine/engine.hpp>

#include <imgui.h>

namespace omega_editor
{

HierarchyPanel::HierarchyPanel(omega::Engine& engine, omega::Entity& selected_entity, omega::Entity& excluded_entity)
    : m_engine(engine), m_selected_entity(selected_entity), m_excluded_entity(excluded_entity)
{}

const char* HierarchyPanel::title() const
{
    return "Hierarchy";
}

void HierarchyPanel::render()
{
    {
        omega::HierarchySystem& hierarchy = m_engine.hierarchy();

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Entities");

        ImGui::SameLine();

        const float button_width = ImGui::GetFrameHeight();
        ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - button_width);

        if (ImGui::Button("+", ImVec2(button_width, 0.0f)))
            m_engine.entities().create_entity();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Entity tree
        for (omega::Entity entity : m_engine.entities())
        {
            if (!hierarchy.has_parent(entity))
                render_entity(entity, hierarchy, m_selected_entity);
        }

        // Empty-space drop target
        const ImVec2 available = ImGui::GetContentRegionAvail();

        ImGui::InvisibleButton("HierarchyDropTarget", ImVec2(available.x, available.y));

        // Deselect when clicking empty space
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            m_selected_entity = omega::Entity{};

        // Allow dropping entities to make them roots
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
            {
                const auto* dragged_entity = static_cast<const omega::Entity*>(payload->Data);

                hierarchy.clear_parent(*dragged_entity);
            }

            ImGui::EndDragDropTarget();
        }
    }

    if (m_pending_delete.has_value())
    {
        m_engine.entities().destroy_entity(*m_pending_delete);
        m_pending_delete.reset();
    }
}

void HierarchyPanel::render_entity(omega::Entity entity, omega::HierarchySystem& hierarchy,
                                   omega::Entity& selected_entity)
{
    if (entity == m_excluded_entity)
        return;

    const auto& children = hierarchy.get_children(entity);

    ImGui::PushID(static_cast<int>(entity.index));
    ImGui::PushID(static_cast<int>(entity.generation));

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;

    if (children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    if (selected_entity == entity)
        flags |= ImGuiTreeNodeFlags_Selected;

    const bool opened = ImGui::TreeNodeEx("Entity", flags, "Entity %u", entity.index);

    if (ImGui::IsItemClicked())
        selected_entity = entity;

    if (ImGui::BeginPopupContextItem("EntityContextMenu"))
    {
        if (ImGui::MenuItem("Delete"))
            m_pending_delete = entity;

        ImGui::EndPopup();
    }

    // Drag source
    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("ENTITY", &entity, sizeof(omega::Entity));

        ImGui::Text("Entity %u", entity.index);
        ImGui::EndDragDropSource();
    }

    // Drop target
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
        {
            const auto* dragged_entity = static_cast<const omega::Entity*>(payload->Data);

            if (*dragged_entity != entity)
                hierarchy.set_parent(*dragged_entity, entity);
        }

        ImGui::EndDragDropTarget();
    }

    if (opened && !children.empty())
    {
        for (omega::Entity child : children)
            render_entity(child, hierarchy, selected_entity);

        ImGui::TreePop();
    }

    ImGui::PopID();
    ImGui::PopID();
}

} // namespace omega_editor