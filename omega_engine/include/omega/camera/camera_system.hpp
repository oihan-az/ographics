#ifndef OMEGA_CAMERA_SYSTEM_HPP
#define OMEGA_CAMERA_SYSTEM_HPP

#include <omega/core/entity/entity_data_cache.hpp>
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
    ~CameraSystem();

    CameraData get_camera_data(Entity entity);

  private:
    void invalidate(Entity entity);
    void on_perspective_camera_event(const ComponentEvent<PerspectiveCamera>& event);
    void on_orthogonal_camera_event(const ComponentEvent<OrthogonalCamera>& event);
    void on_world_transform_changed(const WorldTransformChangedEvent& event);

  private:
    EntityDataCache<CameraData> m_cache;

    Scene& m_scene;
    TransformSystem& m_transforms;
    EventSystem& m_events;

    uint64_t m_perspective_camera_subscription;
    uint64_t m_orthogonal_camera_subscription;
    uint64_t m_world_transform_subscription;
};

} // namespace omega

#endif // OMEGA_CAMERA_SYSTEM_HPP