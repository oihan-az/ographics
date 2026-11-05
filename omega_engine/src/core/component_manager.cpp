#include <omega/core/component_manager.hpp>

namespace omega
{

void ComponentManager::clear()
{
    for (auto& [type, storage] : m_storages)
        storage->clear();
}

} // namespace omega