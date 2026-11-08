#include <omega/core/hierarchy/hierarchy_system.hpp>
#include <omega/core/hierarchy/hierarchy_event.hpp>
#include <omega/core/entity/entity_system.hpp>
#include <omega/core/entity/entity_event.hpp>
#include <omega/core/event/event_system.hpp>

#include <unordered_set>

namespace omega
{

uint64_t entity_unique_key(Entity entity) { return (static_cast<uint64_t>(entity.generation) << 32) | entity.index; }

HierarchySystem::HierarchySystem(EventSystem& events, EntitySystem& entities) : m_events(events), m_entities(entities) {}

void HierarchySystem::update()
{
    // Apply parent changes in chronological order
    const auto& parent_changes = m_events.get_queue<HierarchyParentChangedEvent>().get_events();
    for (const auto& event : parent_changes)
    {
        PendingParentChange change{event.child, event.parent, event.parent.is_valid()};
        apply_parent_change(change);
    }

    // Clear pending changes after processing
    m_pending_parent_changes.clear();

    // Process entity removed
    const auto& removed_queue = m_events.get_queue<EntityRemovedEvent>().get_events();
    if (!removed_queue.empty())
    {
        std::unordered_set<uint64_t> removed_entities;
        for (const auto& event : removed_queue)
            removed_entities.insert(entity_unique_key(event.entity));

        // Process each removed entity
        for (const auto& event : removed_queue)
        {
            const Entity destroyed = event.entity;
            const Entity old_parent = get_parent(destroyed);

            // Find the nearest ancestor that survives this update
            Entity surviving_parent = old_parent;
            std::unordered_set<uint64_t> visited;
            while (surviving_parent.is_valid())
            {
                const uint64_t key = entity_unique_key(surviving_parent);

                if (!visited.insert(key).second)
                {
                    surviving_parent = Entity::invalid();
                    break;
                }

                if (!removed_entities.contains(key))
                    break;

                surviving_parent = get_parent(surviving_parent);
            }

            // Move surviving children out before erasing the destroyed node
            std::vector<Entity> children;
            if (auto* children_ptr = m_children.get(destroyed))
                children = *children_ptr;

            for (Entity child : children)
            {
                m_parents.erase(child);

                if (surviving_parent.is_valid() && m_entities.is_alive(surviving_parent))
                {
                    m_parents.insert(child, surviving_parent);
                    add_child_to_parent(child, surviving_parent);
                }

                m_events.publish(HierarchyModifiedEvent{child});
            }

            // Remove the destroyed entity from its own parent's child list
            if (old_parent.is_valid())
                remove_child_from_parent(destroyed, old_parent);

            m_children.erase(destroyed);
            m_parents.erase(destroyed);
        }
    }
}

bool HierarchySystem::set_parent(Entity child, Entity parent)
{
    if (!m_entities.is_alive(child) || !m_entities.is_alive(parent))
        return false;

    if (child == parent)
        return false;

    if (get_effective_parent(child) == parent)
        return true;

    if (would_create_cycle(child, parent))
        return false;

    PendingParentChange change{child, parent, true};

    m_pending_parent_changes.insert(child, change);
    m_events.publish(HierarchyParentChangedEvent{child, parent});

    return true;
}

bool HierarchySystem::clear_parent(Entity child)
{
    if (!m_entities.is_alive(child))
        return false;

    if (!get_effective_parent(child).is_valid())
        return true;

    PendingParentChange change{child, Entity::invalid(), false};

    m_pending_parent_changes.insert(child, change);
    m_events.publish(HierarchyParentChangedEvent{child, Entity::invalid()});

    return true;
}

bool HierarchySystem::has_parent(Entity entity) const
{
    return m_parents.contains(entity);
}

Entity HierarchySystem::get_parent(Entity entity) const
{
    const auto* parent = m_parents.get(entity);
    if (!parent)
        return Entity::invalid();

    return *parent;
}

const std::vector<Entity>& HierarchySystem::get_children(Entity entity) const
{
    static const std::vector<Entity> empty;

    const auto* children = m_children.get(entity);
    if (!children)
        return empty;

    return *children;
}

Entity HierarchySystem::get_effective_parent(Entity entity) const
{
    if (const auto* pending = m_pending_parent_changes.get(entity))
    {
        if (pending->child == entity)
            return pending->has_parent ? pending->parent : Entity::invalid();
    }

    return get_parent(entity);
}

bool HierarchySystem::would_create_cycle(Entity child, Entity parent) const
{
    std::unordered_set<uint64_t> visited;

    Entity ancestor = parent;

    while (ancestor.is_valid())
    {
        if (ancestor == child)
            return true;

        if (!visited.insert(entity_unique_key(ancestor)).second)
            return true;

        if (!m_entities.is_alive(ancestor))
            return true;

        ancestor = get_effective_parent(ancestor);
    }

    return false;
}

void HierarchySystem::remove_child_from_parent(Entity child, Entity parent)
{
    auto* siblings = m_children.get(parent);
    if (!siblings)
        return;

    std::erase(*siblings, child);

    if (siblings->empty())
        m_children.erase(parent);
}

void HierarchySystem::add_child_to_parent(Entity child, Entity parent)
{
    auto* siblings = m_children.get(parent);
    if (siblings)
    {
        if (std::find(siblings->begin(), siblings->end(), child) == siblings->end())
            siblings->push_back(child);
    }
    else
    {
        m_children.insert(parent, std::vector<Entity>{child});
    }
}

void HierarchySystem::remove_relationship(Entity child)
{
    const Entity parent = get_parent(child);

    if (!parent.is_valid())
        return;

    remove_child_from_parent(child, parent);
    m_parents.erase(child);
}

void HierarchySystem::apply_parent_change(const PendingParentChange& change)
{
    const Entity child = change.child;

    if (!m_entities.is_alive(child))
        return;

    const Entity new_parent = change.has_parent ? change.parent : Entity::invalid();

    if (new_parent.is_valid() && !m_entities.is_alive(new_parent))
        return;

    const Entity old_parent = get_parent(child);

    if (old_parent == new_parent)
        return;

    if (old_parent.is_valid())
        remove_child_from_parent(child, old_parent);

    if (new_parent.is_valid())
    {
        if (auto* current_parent = m_parents.get(child))
            *current_parent = new_parent;
        else
            m_parents.insert(child, new_parent);

        add_child_to_parent(child, new_parent);
    }
    else
    {
        m_parents.erase(child);
    }

    m_events.publish(HierarchyModifiedEvent{child});
}

} // namespace omega