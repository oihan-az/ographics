#ifndef OMEGA_EDITOR_PANEL_CONTENT_HPP
#define OMEGA_EDITOR_PANEL_CONTENT_HPP

namespace omega_editor
{

class PanelContent
{
  public:
    virtual ~PanelContent() = default;

    virtual const char* title() const = 0;
    virtual void render() = 0;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_PANEL_CONTENT_HPP