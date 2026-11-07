#ifndef OMEGA_ENTITY_DATA_CACHE_TPP
#define OMEGA_ENTITY_DATA_CACHE_TPP

namespace omega
{

template <typename T>
bool EntityDataCache<T>::contains(Entity entity) const
{
    return m_set.contains(entity);
}

template <typename T>
bool EntityDataCache<T>::is_dirty(Entity entity) const
{
    if (const auto* entry = m_set.get(entity))
        return entry->dirty;
    return true;
}

template <typename T>
T* EntityDataCache<T>::get(Entity entity)
{
    if (auto* entry = m_set.get(entity))
        return &entry->data;
    return nullptr;
}

template <typename T>
const T* EntityDataCache<T>::get(Entity entity) const
{
    if (auto* entry = m_set.get(entity))
        return &entry->data;
    return nullptr;
}

template <typename T>
void EntityDataCache<T>::set(Entity entity, T data)
{
    if (auto* entry = m_set.get(entity))
    {
        entry->data = std::move(data);
        entry->dirty = false;
    }
    else
    {
        m_set.insert(entity, CacheEntry{std::move(data), false});
    }
}

template <typename T>
void EntityDataCache<T>::invalidate(Entity entity)
{
    if (auto* entry = m_set.get(entity))
        entry->dirty = true;
}

template <typename T>
void EntityDataCache<T>::erase(Entity entity)
{
    m_set.erase(entity);
}

template <typename T>
void EntityDataCache<T>::clear()
{
    m_set.clear();
}

} // namespace omega

#endif // OMEGA_ENTITY_DATA_CACHE_TPP