#include <omega/core/component/component_manager.hpp>

namespace omega
{

void ComponentManager::clear_all()
{
    for (auto& [type, storage] : m_storages)
        storage->clear();
}

void ComponentManager::remove_all(Entity entity)
{
    for (auto& [_, storage] : m_storages)
    {
        storage->remove(entity);
    }
}

void ComponentManager::remove_all(Entity entity, EventSystem& events)
{
    for (auto& [_, storage] : m_storages)
    {
        if (storage->remove(entity))
            storage->publish_removed(entity, events);
    }
}

} // namespace omega