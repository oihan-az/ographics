#ifndef OMEGA_COMPONENT_STORAGE_TPP
#define OMEGA_COMPONENT_STORAGE_TPP

#include <omega/core/event/event_system.hpp>
#include <omega/core/component/component_event.hpp>

#include <utility>

namespace omega
{

template<typename T>
ComponentStorage<T>::ComponentStorage(EventSystem& events) : m_events(events) {}

template<typename T>
bool ComponentStorage<T>::add(Entity entity, T component)
{
    if (m_set.contains_key(entity))
        return false;

    m_set.insert(entity, std::move(component));

    m_events.publish(ComponentAddedEvent<T>{entity});

    return true;
}

template<typename T>
bool ComponentStorage<T>::remove(Entity entity)
{
    if (!m_set.contains(entity))
        return false;

    SparseSetSwapInfo<Entity> swap_info = m_set.erase(entity);

    m_events.publish(ComponentRemovedEvent<T>{entity, swap_info});

    return true;
}

template <typename T> 
bool ComponentStorage<T>::mark_dirty(Entity entity)
{
    if (!m_set.contains(entity))
        return false;

    m_events.publish(ComponentModifiedEvent<T>{entity});

    return true;
}

template<typename T>
bool ComponentStorage<T>::contains(Entity entity) const
{
    return m_set.contains(entity);
};

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
std::vector<Entity> ComponentStorage<T>::entities() const
{
    std::vector<Entity> result;
    result.reserve(m_set.size());

    for (uint32_t i = 0; i < m_set.size(); ++i)
        result.push_back(m_set.key_at(i));

    return result;
}

} // namespace omega

#endif // OMEGA_COMPONENT_STORAGE_TPP