#include <omega/core/component/component_system.hpp>
#include <omega/core/component/component_event.hpp>
#include <omega/core/entity/entity_event.hpp>

namespace omega
{

ComponentSystem::ComponentSystem(EventSystem& events, EntitySystem& entities) : m_events(events), m_entities(entities) {}

void ComponentSystem::update()
{
    // Process removed
    const auto& removed_queue = m_events.get_queue<EntityRemovedEvent>().get_events();
    if (!removed_queue.empty())
    {
        for (const EntityRemovedEvent& event : removed_queue)
        {
            for (auto& [_, storage] : m_storages)
            {
                storage->remove(event.entity);
            }
        }
    }
}

} // namespace omega