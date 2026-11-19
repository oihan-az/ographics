#include <omega_editor/panels/hierarchy_panel.hpp>

#include <omega/core/entity/entity_system.hpp>
#include <omega/core/hierarchy/hierarchy_system.hpp>
#include <omega/engine/engine.hpp>

#include <imgui.h>

namespace omega_editor
{

void HierarchyPanel::render(omega::Engine& engine, omega::Entity& selected_entity)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    const float viewport_width = viewport->WorkSize.x;
    const float viewport_height = viewport->WorkSize.y;

    const float min_width = viewport_width * 0.15f;
    const float max_width = viewport_width * 0.35f;

    m_width_ratio = std::clamp(m_width_ratio, 0.15f, 0.35f);

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(viewport_width * m_width_ratio, viewport_height));

    ImGui::SetNextWindowSizeConstraints(ImVec2(min_width, viewport_height), ImVec2(max_width, viewport_height));

    ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    // Toolbar
    omega::HierarchySystem& hierarchy = engine.hierarchy();

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Entities");

    ImGui::SameLine();

    const float button_width = ImGui::GetFrameHeight();
    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - button_width);

    if (ImGui::Button("+", ImVec2(button_width, 0.0f)))
        engine.entities().create_entity();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Entity tree
    for (omega::Entity entity : engine.entities())
    {
        if (!hierarchy.has_parent(entity))
            render_entity(entity, hierarchy, selected_entity);
    }

    // Empty-space drop target
    const ImVec2 available = ImGui::GetContentRegionAvail();
    ImGui::InvisibleButton("HierarchyDropTarget", ImVec2(available.x, available.y));
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
        {
            const auto* dragged_entity = static_cast<const omega::Entity*>(payload->Data);

            hierarchy.clear_parent(*dragged_entity);
        }

        ImGui::EndDragDropTarget();
    }

    // Update width ratio after resizing
    m_width_ratio = ImGui::GetWindowWidth() / viewport_width;

    ImGui::End();

    if (m_pending_delete.has_value())
    {
        engine.entities().destroy_entity(*m_pending_delete);
        m_pending_delete.reset();
    }
}

void HierarchyPanel::render_entity(omega::Entity entity, omega::HierarchySystem& hierarchy, omega::Entity& selected_entity)
{
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