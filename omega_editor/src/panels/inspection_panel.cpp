#include <omega_editor/panels/inspection_panel.hpp>
#include <omega_editor/component_editor/component_editor_registry.hpp>
#include <omega_editor/layout/panel_slot.hpp>

#include <omega/engine/engine.hpp>

#include <imgui.h>

namespace
{

using ComponentType = std::type_index;

const char* component_name(ComponentType type, omega_editor::ComponentEditorRegistry& registry)
{
    for (const auto& editor : registry.editors())
    {
        if (editor.type == type)
            return editor.name.c_str();
    }

    return type.name();
}

std::vector<ComponentType> affected_components(ComponentType type, omega::Entity entity,
                                               omega::ComponentSystem& components, bool adding)
{
    const auto& graph = components.dependency_graph();

    const auto order = adding ? graph.dependency_order(type) : graph.removal_order(type);

    std::vector<ComponentType> result;

    for (const auto& current : order)
    {
        if (current == type)
            continue;

        const auto* node = graph.find(current);

        if (!node)
            continue;

        const bool exists = node->operations.has(entity);

        if (adding && !exists)
            result.push_back(current);

        if (!adding && exists)
            result.push_back(current);
    }

    return result;
}

std::optional<ComponentType> draw_component_relationships(const omega_editor::ComponentEditor& editor,
                                                          omega::Entity entity, omega::ComponentSystem& components,
                                                          omega_editor::ComponentEditorRegistry& registry)
{
    const auto& graph = components.dependency_graph();
    const auto* node = graph.find(editor.type);

    if (!node)
        return std::nullopt;

    std::optional<ComponentType> navigation_target;

    ImGui::PushID("Relationships");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::TreeNodeEx("Relationships"))
    {
        ImGui::Spacing();

        const ImVec4 dependency_color(0.40f, 0.65f, 0.95f, 1.0f);
        const ImVec4 dependant_color(0.75f, 0.55f, 0.95f, 1.0f);

        auto draw_list =
            [&](const char* label, const std::vector<ComponentType>& types, const ImVec4& color, const char* indicator)
        {
            ImGui::PushID(label);

            ImGui::TextDisabled("%s", label);
            ImGui::Spacing();

            bool found = false;

            for (const auto& type : types)
            {
                const auto* related = graph.find(type);

                if (!related || !related->operations.has(entity))
                    continue;

                found = true;

                ImGui::PushID(type.hash_code());

                const float row_height = ImGui::GetFrameHeight() + 4.0f;

                // Original subtle background with hover feedback.
                ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));

                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));

                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgActive));

                // Full-width clickable row.
                if (ImGui::Selectable("##RelationshipRow", false, ImGuiSelectableFlags_None, ImVec2(0.0f, row_height)))
                {
                    navigation_target = type;
                }

                ImGui::PopStyleColor(3);

                // Draw the original colored indicator and component name
                // over the selectable background.
                const ImVec2 row_min = ImGui::GetItemRectMin();
                const ImVec2 row_max = ImGui::GetItemRectMax();

                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                const float text_y = row_min.y + (row_max.y - row_min.y - ImGui::GetTextLineHeight()) * 0.5f;

                const float padding = 10.0f;

                const ImVec2 indicator_pos(row_min.x + padding, text_y);

                draw_list->AddText(indicator_pos, ImGui::GetColorU32(color), indicator);

                const float indicator_width = ImGui::CalcTextSize(indicator).x;

                const ImVec2 name_pos(indicator_pos.x + indicator_width + 10.0f, text_y);

                draw_list->AddText(name_pos, ImGui::GetColorU32(ImGuiCol_Text), component_name(type, registry));

                ImGui::Spacing();
                ImGui::PopID();
            }

            if (!found)
                ImGui::TextDisabled("  None");

            ImGui::PopID();
        };

        draw_list("Dependencies", node->dependencies, dependency_color, ">");

        ImGui::Spacing();

        draw_list("Dependants", node->dependants, dependant_color, "<");

        ImGui::TreePop();
    }

    ImGui::PopID();

    return navigation_target;
}

omega_editor::ComponentEditor* find_component_editor(std::type_index type, omega_editor::ComponentEditorRegistry& registry)
{
    for (omega_editor::ComponentEditor& editor : registry.editors())
    {
        if (editor.type == type)
            return &editor;
    }

    return nullptr;
}

enum class ConfirmationResult
{
    None,
    Confirmed,
    Cancelled
};

ConfirmationResult draw_component_confirmation(const omega_editor::ComponentEditor& editor, omega::Entity entity,
                                               omega::ComponentSystem& components,
                                               omega_editor::ComponentEditorRegistry& registry, bool adding)
{
    constexpr float popup_width = 420.0f;

    ImGui::SetNextWindowSize(ImVec2(popup_width, 0.0f), ImGuiCond_Appearing);

    if (!ImGui::BeginPopupModal("Confirm component change", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return ConfirmationResult::None;
    }

    const auto affected = affected_components(editor.type, entity, components, adding);

    const ImVec4 accent = adding ? ImVec4(0.30f, 0.75f, 0.50f, 1.0f) : ImVec4(0.90f, 0.35f, 0.35f, 1.0f);

    ConfirmationResult result = ConfirmationResult::None;

    // Header
    ImGui::TextColored(accent, "%s", adding ? "Add component" : "Remove component");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Description
    ImGui::TextDisabled("%s", adding ? "Adding" : "Removing");

    ImGui::SameLine(0.0f, 5.0f);

    ImGui::TextColored(accent, "%s", editor.name.c_str());

    ImGui::Spacing();

    ImGui::TextDisabled("%s", adding ? "requires the following additional components:"
                                     : "will also remove the following components:");

    ImGui::Spacing();

    // Affected components
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));

    const float row_height = ImGui::GetTextLineHeightWithSpacing() + 8.0f;

    const float list_height = std::min(row_height * static_cast<float>(affected.size()) + 12.0f, 220.0f);

    if (ImGui::BeginChild("AffectedComponents", ImVec2(0.0f, list_height), ImGuiChildFlags_Borders))
    {
        for (const auto& type : affected)
        {
            ImGui::Spacing();

            ImGui::TextColored(accent, "%s", adding ? "+" : "-");

            ImGui::SameLine(0.0f, 10.0f);

            ImGui::TextUnformatted(component_name(type, registry));
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Spacing();

    ImGui::TextDisabled("%s", adding ? "Missing dependencies will be created automatically."
                                     : "These components will be removed from this entity.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Footer
    const char* confirm_label = adding ? "Add components" : "Remove components";

    const ImGuiStyle& style = ImGui::GetStyle();

    const float cancel_width = ImGui::CalcTextSize("Cancel").x + style.FramePadding.x * 2.0f;

    const float confirm_width = ImGui::CalcTextSize(confirm_label).x + style.FramePadding.x * 2.0f;

    const float total_width = cancel_width + confirm_width + style.ItemSpacing.x;

    const float available_width = ImGui::GetContentRegionAvail().x;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(0.0f, available_width - total_width));

    if (ImGui::Button("Cancel"))
    {
        result = ConfirmationResult::Cancelled;
        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(accent.x * 0.65f, accent.y * 0.65f, accent.z * 0.65f, 1.0f));

    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, accent);

    ImGui::PushStyleColor(ImGuiCol_ButtonActive, accent);

    if (ImGui::Button(confirm_label))
    {
        result = ConfirmationResult::Confirmed;
        ImGui::CloseCurrentPopup();
    }

    ImGui::PopStyleColor(3);

    ImGui::EndPopup();

    return result;
}

} // namespace

namespace omega_editor
{

InspectionPanel::InspectionPanel(omega::Engine& engine, omega::Entity& selected_entity, ComponentEditorRegistry& registry)
    : m_engine(engine), m_selected_entity(selected_entity), m_registry(registry)
{
}

const char* InspectionPanel::title() const
{
    return "Inspector";
}

void InspectionPanel::render()
{
    {
        omega::ComponentSystem& components = m_engine.components();

        const bool valid_selection = m_selected_entity.is_valid() && m_engine.entities().is_alive(m_selected_entity);

        if (!valid_selection)
        {
            ImGui::TextDisabled("No entity selected");

            // Discard actions associated with a previous selection
            clear_pending_action();
        }
        else
        {
            ImGui::Text("Entity %u", m_selected_entity.index);
            ImGui::Separator();

            // Existing components
            const auto navigation_target = m_navigation_target;
            m_navigation_target.reset();
            for (ComponentEditor& editor : m_registry.editors())
            {
                if (!editor.has(m_selected_entity, components))
                    continue;

                ImGui::PushID(editor.name.c_str());

                const float button_width = ImGui::GetFrameHeight();

                if (ImGui::BeginTable("ComponentHeader", 2))
                {
                    ImGui::TableSetupColumn("Header", ImGuiTableColumnFlags_WidthStretch);

                    ImGui::TableSetupColumn("Remove", ImGuiTableColumnFlags_WidthFixed, button_width);

                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);

                    // Expand the target component
                    if (navigation_target == editor.type)
                    {
                        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                    }

                    const bool opened = ImGui::CollapsingHeader(editor.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

                    ImGui::TableSetColumnIndex(1);

                    if (ImGui::Button("X", ImVec2(button_width, button_width)))
                    {
                        m_pending_action = ComponentAction::Remove;
                        m_pending_type = editor.type;
                    }

                    ImGui::EndTable();

                    if (navigation_target == editor.type)
                    {
                        ImGui::SetScrollHereY(0.2f);
                    }

                    if (opened)
                    {
                        // Draw properties
                        editor.draw(m_selected_entity, components);

                        // Draw relationships and receive any navigation request
                        if (auto target = draw_component_relationships(editor, m_selected_entity, components,
                                                                       m_registry))
                        {
                            m_navigation_target = *target;
                        }
                    }
                }

                ImGui::PopID();
            }

            // Add component
            ImGui::Separator();

            if (ImGui::Button("Add Component", ImVec2(-1.0f, 0.0f)))
            {
                ImGui::OpenPopup("AddComponentPopup");
            }

            if (ImGui::BeginPopup("AddComponentPopup"))
            {
                for (ComponentEditor& editor : m_registry.editors())
                {
                    if (editor.has(m_selected_entity, components))
                        continue;

                    if (ImGui::MenuItem(editor.name.c_str()))
                    {
                        m_pending_action = ComponentAction::Add;
                        m_pending_type = editor.type;
                    }
                }

                ImGui::EndPopup();
            }
        }

        ComponentEditor* editor =
            m_pending_type.has_value() ? find_component_editor(*m_pending_type, m_registry) : nullptr;

        // Evaluate pending operation
        if (valid_selection && m_pending_action != ComponentAction::None && editor != nullptr && !m_confirmation_open)
        {
            const bool adding = m_pending_action == ComponentAction::Add;

            // Lazily register its component type and dependencies
            if (adding && editor->prepare)
                editor->prepare(components);

            const auto affected = affected_components(editor->type, m_selected_entity, components, adding);

            if (affected.empty())
            {
                // No cascading effect: perform immediately
                if (adding)
                    editor->create(m_selected_entity, components);
                else
                    editor->remove(m_selected_entity, components);

                clear_pending_action();
            }
            else
            {
                // Other components are affected: ask for confirmation
                ImGui::OpenPopup("Confirm component change");
                m_confirmation_open = true;
            }
        }

        // Confirmation popup
        editor = m_pending_type.has_value() ? find_component_editor(*m_pending_type, m_registry) : nullptr;
        if (valid_selection && editor != nullptr && m_pending_action != ComponentAction::None && m_confirmation_open)
        {
            const bool adding = m_pending_action == ComponentAction::Add;

            const ConfirmationResult result =
                draw_component_confirmation(*editor, m_selected_entity, components, m_registry, adding);
            if (result == ConfirmationResult::Confirmed)
            {
                if (adding)
                    editor->create(m_selected_entity, components);
                else
                    editor->remove(m_selected_entity, components);

                clear_pending_action();
            }
            else if (result == ConfirmationResult::Cancelled)
            {
                clear_pending_action();
            }
        }

        if (m_confirmation_open && !ImGui::IsPopupOpen("Confirm component change"))
            clear_pending_action();
    }
}

void InspectionPanel::clear_pending_action()
{
    m_pending_action = ComponentAction::None;
    m_pending_type.reset();
    m_confirmation_open = false;
}

} // namespace omega_editor