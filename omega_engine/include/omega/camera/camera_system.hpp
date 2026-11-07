#ifndef OMEGA_CAMERA_SYSTEM_HPP
#define OMEGA_CAMERA_SYSTEM_HPP

#include <omega/core/container/dense_indexed_cache.hpp>
#include <omega/core/event/event_system.hpp>
#include <omega/core/component/component_event.hpp>
#include <omega/core/scene/scene.hpp>

#include <omega/camera/camera.hpp>
#include <omega/transform/transform_system.hpp>
#include <omega/transform/transform_event.hpp>

namespace omega
{

struct CameraData
{
    Matrix4 view{1.0f};
    Matrix4 projection{1.0f};
    Matrix4 view_projection{1.0f};
};

class CameraSystem
{
  public:
    explicit CameraSystem(Scene& scene, EventSystem& events, TransformSystem& transforms);

    void update();

    CameraData get_camera_data(Entity entity);

  private:
    void invalidate(Entity entity);

  private:
    DenseIndexedCache<CameraData> m_perspective_cache;
    DenseIndexedCache<CameraData> m_orthogonal_cache;

    Scene& m_scene;
    TransformSystem& m_transforms;
    EventSystem& m_events;
};

} // namespace omega

#endif // OMEGA_CAMERA_SYSTEM_HPP