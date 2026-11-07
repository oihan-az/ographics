#ifndef OMEGA_COMPONENT_STORAGE_TPP
#define OMEGA_COMPONENT_STORAGE_TPP

#include <omega/core/event/event_system.hpp>
#include <omega/core/component/component_event.hpp>

#include <utility>

namespace omega
{

template<typename T>
bool ComponentStorage<T>::add(Entity entity, T component)
{
    if (!entity.is_valid())
        return false;

    if (m_set.contains_key(entity))
        return false;

    m_set.insert(entity, std::move(component));
    return true;
}

template<typename T>
bool ComponentStorage<T>::remove(Entity entity)
{
    if (!m_set.contains(entity))
        return false;

    m_set.erase(entity);
    return true;
}

template<typename T>
bool ComponentStorage<T>::remove(Entity entity, SparseSetSwapInfo<Entity>& swap_info)
{
    if (!m_set.contains(entity))
        return false;

    swap_info = m_set.erase(entity);
    return true;
}

template<typename T>
bool ComponentStorage<T>::contains(Entity entity) const
{
    return m_set.contains(entity);
};

template <typename T> 
void ComponentStorage<T>::clear()
{
    m_set.clear();
}

template<typename T>
uint32_t ComponentStorage<T>::get_index(Entity entity)
{
    return m_set.get_dense_index(entity);
}

template<typename T>
T* ComponentStorage<T>::get(Entity entity)
{
    return m_set.get(entity);
}

template<typename T>
const T* ComponentStorage<T>::get(Entity entity) const
{
    return m_set.get(entity);
}

template <typename T> 
uint32_t ComponentStorage<T>::size() const
{
    return m_set.size();
}

template <typename T> 
Entity ComponentStorage<T>::entity_at(uint32_t index) const
{
    return m_set.key_at(index);
}

template <typename T>
void ComponentStorage<T>::publish_added(Entity entity, EventSystem& events)
{
    events.publish(ComponentAddedEvent<T>{entity});
}

template <typename T>
void ComponentStorage<T>::publish_removed(Entity entity, const SparseSetSwapInfo<Entity>& swap_info, EventSystem& events)
{
    events.publish(ComponentRemovedEvent<T>{entity, swap_info});
}

template <typename T>
void ComponentStorage<T>::publish_changed(Entity entity, EventSystem& events)
{

    events.publish(ComponentModifiedEvent<T>{entity});
}

} // namespace omega

#endif // OMEGA_COMPONENT_STORAGE_TPP