#ifndef OMEGA_COMPONENT_SYSTEM_TPP
#define OMEGA_COMPONENT_SYSTEM_TPP

#include <omega/core/event/event_system.hpp>

#include <utility>

namespace omega
{

template <typename T> 
ComponentStorage<T>& ComponentSystem::get_storage()
{
    std::type_index type = typeid(T);

    auto it = m_storages.find(type);
    if (it != m_storages.end())
        return *static_cast<ComponentStorage<T>*>(it->second.get());

    auto storage = std::make_unique<ComponentStorage<T>>(m_events);
    ComponentStorage<T>& reference = *storage;

    m_storages.emplace(type, std::move(storage));

    return reference;
}

template <typename T> 
bool ComponentSystem::remove_storage()
{
    if (!has_storage<T>())
        return false;

    return m_storages.erase(std::type_index(typeid(T))) > 0;
}

template <typename T> 
bool ComponentSystem::has_storage() const
{
    return m_storages.contains(std::type_index(typeid(T)));
}

template <typename T> 
bool ComponentSystem::add(Entity entity, T component)
{
    if (!m_entities.is_valid(entity))
        return false;

    return get_storage<T>().add(entity, std::move(component));
}

template <typename T> 
bool ComponentSystem::remove(Entity entity)
{
    if (!has_storage<T>())
        return false;

    return get_storage<T>().remove(entity);
}

template <typename T> 
bool ComponentSystem::mark_dirty(Entity entity)
{
    if (!m_entities.is_valid(entity))
        return false;

    return get_storage<T>().mark_dirty(entity);
}

template<typename T>
bool ComponentSystem::has(Entity entity) const
{
    auto it = m_storages.find(std::type_index(typeid(T)));
    if (it == m_storages.end())
        return false;

    const auto* storage = static_cast<const ComponentStorage<T>*>(it->second.get());
    return storage->contains(entity);
}

template<typename T>
T* ComponentSystem::get(Entity entity)
{
    if (!has_storage<T>())
        return nullptr;

    return get_storage<T>().get(entity);
}

template<typename T>
const T* ComponentSystem::get(Entity entity) const
{
    auto it = m_storages.find(std::type_index(typeid(T)));
    if (it == m_storages.end())
        return nullptr;

    const auto* storage = static_cast<const ComponentStorage<T>*>(it->second.get());
    return storage->get(entity);
}

template <typename T>
std::vector<Entity> ComponentSystem::entities_with() const
{
    auto it = m_storages.find(std::type_index(typeid(T)));
    if (it == m_storages.end())
        return {};

    const auto* storage = static_cast<const ComponentStorage<T>*>(it->second.get());

    return storage->entities();
}

} // namespace omega

#endif // OMEGA_COMPONENT_SYSTEM_TPP