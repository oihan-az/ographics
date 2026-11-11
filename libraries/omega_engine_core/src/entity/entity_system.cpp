#include <omega/core/entity/entity_system.hpp>
#include <omega/core/entity/entity_event.hpp>
#include <omega/core/event/event_system.hpp>

namespace omega
{

EntitySystem::EntitySystem(EventSystem& events) : m_events(events) {}

void EntitySystem::update()
{
    // Process added
    const auto& added_queue = m_events.get_queue<EntityAddedEvent>().get_events();
    if (!added_queue.empty())
    {
        for (auto it = added_queue.rbegin(); it != added_queue.rend(); ++it)
        {
            if (it->entity.index >= m_entity_slots.size())
                continue;

            EntitySlot& slot = m_entity_slots[it->entity.index];

            if (slot.generation != it->entity.generation)
                continue;

            if (slot.state != EntityState::PendingCreation)
                continue;

            ++m_entity_count;

            slot.state = EntityState::Alive;
        }
    }

    // Process removed
    const auto& removed_queue = m_events.get_queue<EntityRemovedEvent>().get_events();
    if (!removed_queue.empty())
    {
        for (auto it = removed_queue.rbegin(); it != removed_queue.rend(); ++it)
        {
            if (it->entity.index >= m_entity_slots.size())
                continue;

            EntitySlot& slot = m_entity_slots[it->entity.index];

            if (slot.generation != it->entity.generation)
                continue;

            if (slot.state != EntityState::PendingDestruction && slot.state != EntityState::PendingCreationDestruction)
                continue;

            if (slot.state == EntityState::PendingDestruction)
                --m_entity_count;

            if (slot.generation == std::numeric_limits<uint32_t>::max())
            {
                slot.state = EntityState::Retired;
            }
            else
            {
                ++slot.generation;
                slot.state = EntityState::Free;
                m_free_indices.push_back(it->entity.index);
            }
        }
    }
}

Entity EntitySystem::create_entity()
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

    m_events.publish(EntityAddedEvent{entity});

    return entity;
}

bool EntitySystem::destroy_entity(Entity entity)
{
    if (!is_valid(entity))
        return false;

    EntitySlot& slot = m_entity_slots[entity.index];

    if (slot.state == EntityState::PendingCreation)
        slot.state = EntityState::PendingCreationDestruction;
    else
        slot.state = EntityState::PendingDestruction;

    m_events.publish(EntityRemovedEvent{entity});

    return true;
}

bool EntitySystem::is_alive(Entity entity) const
{
    if (entity.index >= m_entity_slots.size())
        return false;

    const EntitySlot& slot = m_entity_slots[entity.index];
    return (slot.state == EntityState::Alive) &&
            slot.generation == entity.generation;
}

bool EntitySystem::is_valid(Entity entity) const
{
    if (entity.index >= m_entity_slots.size())
        return false;

    const EntitySlot& slot = m_entity_slots[entity.index];
    return (slot.state == EntityState::Alive || slot.state == EntityState::PendingCreation) &&
           slot.generation == entity.generation;
}

uint32_t EntitySystem::entity_count() const
{
    return m_entity_count;
}

EntityIterator EntitySystem::begin() const
{
    return EntityIterator(m_entity_slots, 0);
}

EntityIterator EntitySystem::end() const
{
    return EntityIterator(m_entity_slots, static_cast<uint32_t>(m_entity_slots.size()));
}

} // namespace omega