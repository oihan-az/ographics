#include <omega_editor/layout/panel.hpp>
#include <omega_editor/layout/panel_content.hpp>

namespace omega_editor
{

Panel::Panel(PanelSlot& slot, std::string title) : m_slot(slot), m_title(std::move(title)) {}

void Panel::set_content(PanelContent& content)
{
    m_content = &content;
}

void Panel::render()
{
    const bool visible = m_slot.begin(m_title.c_str());

    if (visible)
        render_content();

    m_slot.end();
}

void Panel::render_content()
{
    if (m_content)
        m_content->render();
}

} // namespace omega_editor