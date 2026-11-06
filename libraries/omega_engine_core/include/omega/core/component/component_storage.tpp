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

    if (entity.index < m_sparse.size() && m_sparse[entity.index] != Entity::invalid_index)
        return false;

    if (entity.index >= m_sparse.size())
        m_sparse.resize(entity.index + 1, Entity::invalid_index);

    uint32_t dense_index = static_cast<uint32_t>(m_components.size());

    m_entities.push_back(entity);
    m_components.push_back(std::move(component));

    m_sparse[entity.index] = dense_index;

    return true;
}

template<typename T>
bool ComponentStorage<T>::remove(Entity entity)
{
    if (!contains(entity))
        return false;

    uint32_t dense_index = m_sparse[entity.index];
    uint32_t last_index = static_cast<uint32_t>(m_components.size() - 1);

    if (dense_index != last_index)
    {
        m_entities[dense_index] = m_entities[last_index];
        m_components[dense_index] = std::move(m_components[last_index]);

        m_sparse[m_entities[dense_index].index] = dense_index;
    }

    m_entities.pop_back();
    m_components.pop_back();

    m_sparse[entity.index] = Entity::invalid_index;

    return true;
}

template<typename T>
bool ComponentStorage<T>::contains(Entity entity) const
{
    if (entity.index >= m_sparse.size())
        return false;

    uint32_t dense_index = m_sparse[entity.index];

    if (dense_index == Entity::invalid_index)
        return false;

    return m_entities[dense_index] == entity;
};

template <typename T> 
void ComponentStorage<T>::clear()
{
    m_sparse.clear();
    m_entities.clear();
    m_components.clear();
}

template<typename T>
T* ComponentStorage<T>::get(Entity entity)
{
    if (!contains(entity))
        return nullptr;

    return &m_components[m_sparse[entity.index]];
}

template<typename T>
const T* ComponentStorage<T>::get(Entity entity) const
{
    if (!contains(entity))
        return nullptr;

    return &m_components[m_sparse[entity.index]];
}

template <typename T> 
uint32_t ComponentStorage<T>::size() const
{
    return static_cast<uint32_t>(m_components.size());
}

template <typename T> 
auto ComponentStorage<T>::begin()
{
    return m_components.begin();
}

template <typename T> 
auto ComponentStorage<T>::end()
{
    return m_components.end();
}


template <typename T> 
auto ComponentStorage<T>::begin() const
{
    return m_components.begin();
}

template <typename T> 
auto ComponentStorage<T>::end() const
{
    return m_components.end();
}

template <typename T> 
Entity ComponentStorage<T>::entity_at(uint32_t index) const
{
    return m_entities.at(index);
}

template <typename T>
void ComponentStorage<T>::publish_added(Entity entity, EventSystem& events)
{
    events.publish(ComponentEvent<T>{entity, ComponentEventType::Added});
}

template <typename T>
void ComponentStorage<T>::publish_removed(Entity entity, EventSystem& events)
{
    events.publish(ComponentEvent<T>{entity, ComponentEventType::Removed});
}

template <typename T>
void ComponentStorage<T>::publish_changed(Entity entity, EventSystem& events)
{
    events.publish(ComponentEvent<T>{entity, ComponentEventType::Changed});
}

} // namespace omega

#endif // OMEGA_COMPONENT_STORAGE_TPP