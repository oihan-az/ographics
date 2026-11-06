#include <omega/core/scene/scene.hpp>
#include <omega/core/scene/hierarchy_event.hpp>

#include <limits>
#include <algorithm>

namespace omega
{

Scene::Scene(EventSystem& events) : m_events(&events) {}

Entity Scene::create_entity()
{
    uint32_t index;

    if (!m_free_indices.empty())
    {
        index = m_free_indices.back();
        m_free_indices.pop_back();
    }
    else
    {
        index = static_cast<uint32_t>(m_entity_slots.size());
        m_entity_slots.push_back({});
    }

    EntitySlot& slot = m_entity_slots[index];
    slot.state = EntityState::PendingCreation;

    Entity entity{.index = index, .generation = slot.generation};
    m_pending_commands.push_back({.type = EntityCommandType::Create, .entity = entity});
    return entity;
}

bool Scene::destroy_entity(Entity entity) 
{
    if (!is_destructible(entity))
        return false;

    for (const EntityCommand& command : m_pending_commands)
    {
        if (command.type == EntityCommandType::Destroy && command.entity == entity)
        {
            return false;
        }
    }

    m_pending_commands.push_back({.type = EntityCommandType::Destroy, .entity = entity});
    return true;
}

void Scene::flush_commands() 
{
    for (const EntityCommand& command : m_pending_commands)
    {
        EntitySlot& slot = m_entity_slots[command.entity.index];

        if (slot.generation != command.entity.generation)
            continue;

        switch (command.type)
        {
        case EntityCommandType::Create:
        {
            if (slot.state != EntityState::PendingCreation)
                break;

            slot.state = EntityState::Alive;
            ++m_entity_count;
            break;
        }

        case EntityCommandType::Destroy:
        {
            if (slot.state != EntityState::Alive)
                break;

            // Preserve the destroyed entity's parent
            Entity parent = get_parent(command.entity);

            // Detach the entity from its parent
            clear_parent(command.entity);

            // Reparent its children to the preserved parent
            auto children_it = m_children.find(command.entity);
            if (children_it != m_children.end())
            {
                // Move the children out before modifying the hierarchy map
                std::vector<Entity> children = std::move(children_it->second);
                m_children.erase(children_it);

                for (Entity child : children)
                {
                    // Reparent children to the destroyed entity's parent
                    if (parent.is_valid())
                    {
                        m_parents[child] = parent;
                        m_children[parent].push_back(child);
                    }
                    else
                    {
                        // Children become roots when their parent was a root
                        m_parents.erase(child);
                    }

                    // Notify systems that this child's hierarchy changed
                    if (m_events)
                        m_events->publish(HierarchyEvent{child});
                }
            }

            // Remove the entity's components
            if (m_events)
                m_components.remove_all(command.entity, *m_events);
            else
                m_components.remove_all(command.entity);

            if (slot.generation == std::numeric_limits<uint32_t>::max())
            {
                slot.state = EntityState::Retired;
            }
            else
            {
                ++slot.generation;
                slot.state = EntityState::Free;
                m_free_indices.push_back(command.entity.index);
            }

            --m_entity_count;
            break;
        }
        }
    }

    m_pending_commands.clear();
}

bool Scene::is_alive(Entity entity) const
{
    if (entity.index >= m_entity_slots.size())
        return false;

    const EntitySlot& slot = m_entity_slots[entity.index];
    return (slot.state == EntityState::Alive) &&
            slot.generation == entity.generation;
}

bool Scene::is_destructible(Entity entity) const
{
    if (entity.index >= m_entity_slots.size())
        return false;

    const EntitySlot& slot = m_entity_slots[entity.index];
    return (slot.state == EntityState::Alive || slot.state == EntityState::PendingCreation) &&
           slot.generation == entity.generation;
}

uint32_t Scene::entity_count() const
{
    return m_entity_count;
}

EntityIterator Scene::begin() const
{
    return EntityIterator(this, 0);
}

EntityIterator Scene::end() const
{
    return EntityIterator(this, static_cast<uint32_t>(m_entity_slots.size()));
}

bool Scene::set_parent(Entity child, Entity parent)
{
    // Both entities must be alive
    if (!is_alive(child) || !is_alive(parent))
        return false;

    // An entity cannot be its own parent
    if (child == parent)
        return false;

    // Walk up the parent chain to prevent circular hierarchies
    Entity ancestor = parent;
    while (m_parents.contains(ancestor))
    {
        ancestor = m_parents.at(ancestor);

        if (ancestor == child)
            return false;

        if (!is_alive(ancestor))
            return false;
    }

    // Remove the child from its previous parent's child list
    auto previous_parent = m_parents.find(child);
    if (previous_parent != m_parents.end())
    {
        auto& siblings = m_children[previous_parent->second];
        std::erase(siblings, child);
        if (siblings.empty())
            m_children.erase(previous_parent->second);
    }

    // Assign the new parent and update its child list
    m_parents[child] = parent;
    m_children[parent].push_back(child);

    // Notify listeners that the hierarchy changed
    if (m_events)
        m_events->publish(HierarchyEvent{child});

    return true;
}

bool Scene::clear_parent(Entity child)
{
    // The child must be alive
    if (!is_alive(child))
        return false;

    // Nothing to clear if the entity is already a root
    auto parent = m_parents.find(child);
    if (parent == m_parents.end())
        return true;

    // Remove the child from its parent's child list
    auto children = m_children.find(parent->second);
    if (children != m_children.end())
    {
        std::erase(children->second, child);

        if (children->second.empty())
            m_children.erase(children);
    }

    // Remove the parent relationship
    m_parents.erase(parent);

    // Notify listeners that the hierarchy changed
    if (m_events)
        m_events->publish(HierarchyEvent{child});

    return true;
}

bool Scene::has_parent(Entity entity) const
{
    return m_parents.contains(entity);
}

Entity Scene::get_parent(Entity entity) const
{
    // Return an invalid entity when there is no parent
    auto it = m_parents.find(entity);
    if (it == m_parents.end())
        return Entity::invalid();

    return it->second;
}

const std::vector<Entity>& Scene::get_children(Entity entity) const
{
    // Return an empty list when the entity has no children
    static const std::vector<Entity> empty;

    auto it = m_children.find(entity);
    if (it == m_children.end())
        return empty;

    return it->second;
}

} // namespace omega