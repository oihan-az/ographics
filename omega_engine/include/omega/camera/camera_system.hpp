#ifndef OMEGA_CAMERA_SYSTEM_HPP
#define OMEGA_CAMERA_SYSTEM_HPP

#include <omega/core/container/dense_indexed_cache.hpp>
#include <omega/core/entity/entity.hpp>
#include <omega/core/math/matrix4.hpp>

#include <omega/camera/camera.hpp>
#include <omega/transform/transform_system.hpp>
#include <omega/transform/transform_event.hpp>

namespace omega
{

class EventSystem;
class ComponentSystem;
class HierarchySystem;
class EntitySystem;
class TransformSystem;

struct CameraData
{
    Matrix4 view{1.0f};
    Matrix4 projection{1.0f};
    Matrix4 view_projection{1.0f};
};

class CameraSystem
{
  public:
    explicit CameraSystem(EventSystem& events, ComponentSystem& components, 
                          HierarchySystem& hierarchy, EntitySystem& entities,
                          TransformSystem& transforms);

    void update();

    CameraData get_camera_data(Entity entity);

  private:
    void invalidate(Entity entity);

  private:
    DenseIndexedCache<CameraData> m_perspective_cache;
    DenseIndexedCache<CameraData> m_orthogonal_cache;

    TransformSystem& m_transforms;
    ComponentSystem& m_components;
    HierarchySystem& m_hierarchy;
    EntitySystem& m_entitites;
    EventSystem& m_events;
};

} // namespace omega

#endif // OMEGA_CAMERA_SYSTEM_HPP