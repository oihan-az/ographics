#ifndef OMEGA_EDITOR_VIEWPORT_TOOLBAR_HPP
#define OMEGA_EDITOR_VIEWPORT_TOOLBAR_HPP

namespace omega
{

class ComponentSystem;

} // namespace omega

namespace omega_editor
{

class CameraViewport;
class TransformGizmo;

enum class ViewportToolbarAction
{
    None,
    ToggleFullscreen,
    CameraChanged
};

class ViewportToolbar
{
  public:
    ViewportToolbar(omega::ComponentSystem& components, const omega::Entity& editor_camera);

    ViewportToolbarAction render(CameraViewport& viewport, TransformGizmo& gizmo);

    void show();

  private:
    omega::ComponentSystem& m_components;
    const omega::Entity& m_editor_camera;

    void draw_camera_picker(CameraViewport& viewport, bool& changed);
    void draw_transform_tools(TransformGizmo& gizmo);

    bool m_hidden = false;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_VIEWPORT_TOOLBAR_HPP