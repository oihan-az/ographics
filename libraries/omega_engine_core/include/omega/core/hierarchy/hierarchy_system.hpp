#ifndef OMEGA_HIERARCHY_HPP
#define OMEGA_HIERARCHY_HPP

#include <omega/core/entity/entity.hpp>
#include <omega/core/entity/entity_indexer.hpp>
#include <omega/core/container/sparse_set.hpp>

#include <vector>

namespace omega
{

class EventSystem;
class EntitySystem;

class HierarchySystem
{
  public:
    HierarchySystem(EventSystem& events, EntitySystem& entities);

    void update();

    bool set_parent(Entity child, Entity parent);
    bool clear_parent(Entity child);

    bool has_parent(Entity entity) const;

    Entity get_parent(Entity entity) const;
    const std::vector<Entity>& get_children(Entity entity) const;

  private:
    struct PendingParentChange
    {
        Entity child;
        Entity parent;
        bool has_parent;
    };
    SparseSetContainer<PendingParentChange, Entity> m_pending_parent_changes;

    Entity get_effective_parent(Entity entity) const;
    bool would_create_cycle(Entity child, Entity parent) const;

    void remove_child_from_parent(Entity child, Entity parent);
    void add_child_to_parent(Entity child, Entity parent);
    void remove_relationship(Entity child);
    void apply_parent_change(const PendingParentChange& change);

    SparseSetContainer<Entity, Entity> m_parents;
    SparseSetContainer<std::vector<Entity>, Entity> m_children;

    // Events
    EventSystem& m_events;

    // Entities
    EntitySystem& m_entities;
};

} // namespace omega

#endif // OMEGA_HIERARCHY_HPP