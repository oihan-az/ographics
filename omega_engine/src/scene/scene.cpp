#include <omega/scene/scene.hpp>

#include <limits>

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

            auto removed_types = m_components.remove_all(command.entity);
            for (const std::type_index& type : removed_types)
            {
                publish_component_event(command.entity, type, ComponentEventType::Removed);
            }

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

void Scene::publish_component_event(Entity entity, std::type_index component_type, ComponentEventType type)
{
    if (!m_events)
        return;

    m_events->publish(ComponentEvent{entity, component_type, type});
}

} // namespace omega