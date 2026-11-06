#ifndef OMEGA_ENTITY_DATA_CACHE_TPP
#define OMEGA_ENTITY_DATA_CACHE_TPP

namespace omega
{

template <typename T>
bool EntityDataCache<T>::contains(Entity entity) const
{
    return m_entries.contains(entity);
}

template <typename T>
bool EntityDataCache<T>::is_dirty(Entity entity) const
{
    auto it = m_entries.find(entity);
    return it == m_entries.end() || it->second.dirty;
}

template <typename T>
T* EntityDataCache<T>::get(Entity entity)
{
    auto it = m_entries.find(entity);
    return it != m_entries.end() ? &it->second.data : nullptr;
}

template <typename T>
const T* EntityDataCache<T>::get(Entity entity) const
{
    auto it = m_entries.find(entity);
    return it != m_entries.end() ? &it->second.data : nullptr;
}

template <typename T>
void EntityDataCache<T>::set(Entity entity, T data)
{
    auto& entry = m_entries[entity];
    entry.data = std::move(data);
    entry.dirty = false;
}

template <typename T>
void EntityDataCache<T>::invalidate(Entity entity)
{
    auto it = m_entries.find(entity);
    if (it != m_entries.end())
        it->second.dirty = true;
}

template <typename T>
void EntityDataCache<T>::erase(Entity entity)
{
    m_entries.erase(entity);
}

template <typename T>
void EntityDataCache<T>::clear()
{
    m_entries.clear();
}

} // namespace omega

#endif // OMEGA_ENTITY_DATA_CACHE_TPP