#ifndef OMEGA_TRANSFORM_SYSTEM_HPP
#define OMEGA_TRANSFORM_SYSTEM_HPP

#include <omega/core/container/dense_indexed_cache.hpp>
#include <omega/core/entity/entity.hpp>
#include <omega/core/math/matrix4.hpp>

#include <omega/transform/transform.hpp>

#include <unordered_map>
#include <unordered_set>

namespace omega
{

class EventSystem;
class ComponentSystem;
class HierarchySystem;
class EntitySystem;
class HierarchyModifiedEvent;

class TransformSystem
{
  public:
    TransformSystem(EventSystem& events, ComponentSystem& components, HierarchySystem& hierarchy, EntitySystem& entities);

    void prepare_hierarchy_changes();
    void update();

    Matrix4 get_world_matrix(Entity entity);

  private:
    void invalidate(Entity entity);
    void invalidate_cache(Entity entity);
    void mark_hierarchy_dirty(Entity entity);
    void mark_descendants_dirty(Entity entity);

    void on_hierarchy_modified(const HierarchyModifiedEvent& event);

    Matrix4 calculate_world_uncached(Entity entity);
    void apply_hierarchy_changes();
    void apply_hierarchy_change(Entity entity, std::unordered_set<Entity, EntityHash>& processed);

  private:
    DenseIndexedCache<Matrix4> m_cache;

    struct HierarchySnapshot
    {
        Entity entity;
        Entity old_parent;
        Matrix4 world;
    };
    std::unordered_map<Entity, HierarchySnapshot, EntityHash> m_hierarchy_snapshots;

    ComponentSystem& m_components;
    HierarchySystem& m_hierarchy;
    EntitySystem& m_entities;
    EventSystem& m_events;
};

} // namespace omega

#endif // OMEGA_TRANSFORM_SYSTEM_HPP