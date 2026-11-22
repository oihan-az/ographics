#ifndef OMEGA_EDITOR_PANEL_HPP
#define OMEGA_EDITOR_PANEL_HPP

#include <omega_editor/layout/panel_slot.hpp>

#include <string>

namespace omega_editor
{

class PanelContent;

class Panel
{
  public:
    Panel(PanelSlot& slot, std::string title);
    virtual ~Panel() = default;

    void set_content(PanelContent& content);

    void render();

  protected:
    virtual void render_content();

  private:
    PanelSlot& m_slot;
    std::string m_title;

    PanelContent* m_content = nullptr;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_PANEL_HPP