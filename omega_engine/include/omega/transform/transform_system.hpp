#ifndef OMEGA_TRANSFORM_SYSTEM_HPP
#define OMEGA_TRANSFORM_SYSTEM_HPP

#include <omega/core/container/dense_indexed_cache.hpp>
#include <omega/core/entity/entity.hpp>
#include <omega/core/math/matrix4.hpp>

#include <omega/transform/transform.hpp>

namespace omega
{

class EventSystem;
class ComponentSystem;
class HierarchySystem;
class EntitySystem;

class TransformSystem
{
  public:
    TransformSystem(EventSystem& events, ComponentSystem& components, HierarchySystem& hierarchy, EntitySystem& entities);

    void update();

    Matrix4 get_world_matrix(Entity entity);

  private:
    void invalidate(Entity entity);

  private:
    DenseIndexedCache<Matrix4> m_cache;

    ComponentSystem& m_components;
    HierarchySystem& m_hierarchy;
    EntitySystem& m_entitites;
    EventSystem& m_events;
};

} // namespace omega

#endif // OMEGA_TRANSFORM_SYSTEM_HPP