#ifndef OMEGA_EDITOR_CAMERA_CONTROLLER_HPP
#define OMEGA_EDITOR_CAMERA_CONTROLLER_HPP

#include <omega_editor/camera/editor_camera_controls.hpp>

#include <omega/core/entity/entity.hpp>
#include <omega/core/math/vector3.hpp>

namespace omega
{
class Engine;
class Transform;
} // namespace omega

namespace omega_editor
{

class CameraViewport;

enum class CameraNavigationMode
{
    None,
    Fly,
    Orbit,
    Pan,
    Dolly
};

class EditorCameraController
{
  public:
    void update(float delta_time, const CameraViewport& viewport, const EditorCameraControls& controls,
                omega::Engine& engine, omega::Entity editor_camera, omega::Entity selected_entity);

    bool is_navigating() const;

  private:
    void update_navigation_mode(const CameraViewport& viewport);

    bool update_rotation(omega::Transform& transform, const EditorCameraControls& controls);
    bool update_movement(omega::Transform& transform, const EditorCameraControls& controls, float delta_time);
    bool update_orbit(omega::Transform& transform, const EditorCameraControls& controls);
    bool update_pan(omega::Transform& transform, const CameraViewport& viewport, const EditorCameraControls& controls, omega::Engine& engine);
    bool update_dolly(omega::Transform& transform, const CameraViewport& viewport, const EditorCameraControls& controls, omega::Engine& engine, float amount);

    void initialize_pivot(const omega::Transform& transform);
    bool focus_entity(omega::Transform& camera_transform, omega::Engine& engine, omega::Entity selected_entity);

  private:
    CameraNavigationMode m_mode = CameraNavigationMode::None;

    omega::Vector3 m_pivot{0.0f};
    float m_orbit_distance = 5.0f;
    float m_speed_multiplier = 1.0f;

    bool m_pivot_initialized = false;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_CAMERA_CONTROLLER_HPP