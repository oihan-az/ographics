#include <omega/core/component_manager.hpp>

namespace omega
{

void ComponentManager::clear_all()
{
    for (auto& [type, storage] : m_storages)
        storage->clear();
}

void ComponentManager::remove_all(Entity entity)
{
    for (auto& [type, storage] : m_storages)
        storage->remove(entity);
}

} // namespace omega