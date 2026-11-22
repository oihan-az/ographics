#include <omega_editor/layout/tabbed_panel.hpp>
#include <omega_editor/layout/panel_content.hpp>

#include <imgui.h>

#include <algorithm>
#include <utility>

namespace omega_editor
{

TabbedPanel::TabbedPanel(PanelSlot& slot, std::string title) : Panel(slot, std::move(title)) {}

void TabbedPanel::add_content(PanelContent& content)
{
    if (std::find(m_contents.begin(), m_contents.end(), &content) != m_contents.end())
        return;

    m_contents.push_back(&content);
}

void TabbedPanel::render_content()
{
    if (!ImGui::BeginTabBar("##PanelTabs"))
        return;

    for (PanelContent* content : m_contents)
    {
        if (ImGui::BeginTabItem(content->title()))
        {
            content->render();
            ImGui::EndTabItem();
        }
    }

    ImGui::EndTabBar();
}

} // namespace omega_editor