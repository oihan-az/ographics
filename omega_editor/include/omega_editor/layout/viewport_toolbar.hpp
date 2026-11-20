#ifndef OMEGA_EDITOR_VIEWPORT_TOOLBAR_HPP
#define OMEGA_EDITOR_VIEWPORT_TOOLBAR_HPP

namespace omega_editor
{

class CameraViewport;

enum class ViewportToolbarAction
{
    None,
    ToggleFullscreen
};

class ViewportToolbar
{
  public:
    ViewportToolbarAction render(CameraViewport& viewport);

    void show();

  private:
    bool m_hidden = false;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_VIEWPORT_TOOLBAR_HPP