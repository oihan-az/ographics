#ifndef OMEGA_COMPONENT_MANAGER_TPP
#define OMEGA_COMPONENT_MANAGER_TPP

#include <omega/core/event/event_system.hpp>

#include <utility>

namespace omega
{

template <typename T> 
ComponentStorage<T>& ComponentManager::get_storage()
{
    std::type_index type = typeid(T);

    auto it = m_storages.find(type);

    if (it != m_storages.end())
        return *static_cast<ComponentStorage<T>*>(it->second.get());

    auto storage = std::make_unique<ComponentStorage<T>>();
    ComponentStorage<T>& reference = *storage;

    m_storages.emplace(type, std::move(storage));

    return reference;
}

template <typename T> 
bool ComponentManager::remove_storage()
{
    return m_storages.erase(std::type_index(typeid(T))) > 0;
}

template <typename T> 
bool ComponentManager::has_storage() const
{
    return m_storages.contains(std::type_index(typeid(T)));
}

template <typename T> 
bool ComponentManager::add(Entity entity, T component)
{
    return get_storage<T>().add(entity, std::move(component));
}

template <typename T> 
bool ComponentManager::remove(Entity entity)
{
    if (!has_storage<T>())
        return false;

    return get_storage<T>().remove(entity);
}

template<typename T>
T* ComponentManager::get(Entity entity)
{
    if (!has_storage<T>())
        return nullptr;

    return get_storage<T>().get(entity);
}

template<typename T>
const T* ComponentManager::get(Entity entity) const
{
    auto it = m_storages.find(std::type_index(typeid(T)));

    if (it == m_storages.end())
        return nullptr;

    const ComponentStorage<T>* storage = static_cast<const ComponentStorage<T>*>(it->second.get());
    return storage->get(entity);
}

template<typename T>
bool ComponentManager::has(Entity entity) const
{
    auto it = m_storages.find(std::type_index(typeid(T)));

    if (it == m_storages.end())
        return false;

    const ComponentStorage<T>* storage = static_cast<const ComponentStorage<T>*>(it->second.get());
    return storage->contains(entity);
}

template <typename T> 
void ComponentManager::publish_added(Entity entity, EventSystem& events)
{
    if (!has_storage<T>())
        return;

    get_storage<T>().publish_added(entity, events);
}

template <typename T> 
void ComponentManager::publish_removed(Entity entity, EventSystem& events)
{
    if (!has_storage<T>())
        return;

    get_storage<T>().publish_removed(entity, events);
}

template <typename T> 
void ComponentManager::publish_changed(Entity entity, EventSystem& events)
{
    if (!has_storage<T>())
        return;

    get_storage<T>().publish_changed(entity, events);
}

} // namespace omega

#endif // OMEGA_COMPONENT_MANAGER_TPP