#include <omega/core/component/component_manager.hpp>

namespace omega
{

void ComponentManager::clear_all()
{
    for (auto& [type, storage] : m_storages)
        storage->clear();
}

std::vector<std::type_index> ComponentManager::remove_all(Entity entity)
{
    std::vector<std::type_index> removed_types;

    for (auto& [type, storage] : m_storages)
    {
        if (!storage->contains(entity))
            continue;

        if (storage->remove(entity))
            removed_types.push_back(type);
    }

    return removed_types;
}

} // namespace omega