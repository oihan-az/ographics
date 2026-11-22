#ifndef OMEGA_EDITOR_TABBED_PANEL_HPP
#define OMEGA_EDITOR_TABBED_PANEL_HPP

#include <omega_editor/layout/panel.hpp>

#include <vector>

namespace omega_editor
{

class PanelContent;

class TabbedPanel : public Panel
{
  public:
    TabbedPanel(PanelSlot& slot, std::string title);

    void add_content(PanelContent& content);

  protected:
    void render_content() override;

  private:
    std::vector<PanelContent*> m_contents;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_TABBED_PANEL_HPP