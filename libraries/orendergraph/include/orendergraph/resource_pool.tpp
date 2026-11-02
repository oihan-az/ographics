#ifndef ORENDERGRAPH_RESOURCE_POOL_TPP
#define ORENDERGRAPH_RESOURCE_POOL_TPP

namespace orendergraph
{

template <typename T>
ResourcePool<T>::~ResourcePool()
{
    reset();
}

template <typename T> 
void ResourcePool<T>::reset()
{
    for (auto& slot : m_slots)
    {
        if (!slot.is_imported && slot.physical_resource != nullptr)
        {
            delete slot.physical_resource;
            slot.physical_resource = nullptr;
        }
    }
    m_slots.clear();
}

template <typename T>
typename ResourceHandle<T> ResourcePool<T>::add_slot(T* resource, bool is_imported)
{
    uint32_t assigned_id = static_cast<uint32_t>(m_slots.size());
    m_slots.push_back({resource, is_imported});

    ResourceHandle<T> handle;
    handle.id = assigned_id;

    return handle;
}

template <typename T>
void ResourcePool<T>::replace_resource(typename ResourceHandle<T> handle, T* new_resource)
{
    if (!handle.is_valid())
        return;

    auto& resource_slot = m_slots.at(handle.id);

    if (resource_slot.physical_resource == new_resource)
        return;

    if (!resource_slot.is_imported)
        delete resource_slot.physical_resource;

    resource_slot.physical_resource = new_resource;
}

template <typename T>
typename ResourcePool<T>::ResourceSlot& ResourcePool<T>::slot(uint32_t resource_id)
{
    return m_slots.at(resource_id);
}

template <typename T>
const typename ResourcePool<T>::ResourceSlot& ResourcePool<T>::slot(uint32_t resource_id) const
{
    return m_slots.at(resource_id);
}

template <typename T>
uint32_t ResourcePool<T>::slot_count()
{
    return static_cast<uint32_t>(m_slots.size());
}

template <typename T>
typename ResourcePool<T>::StateType ResourcePool<T>::get_current_state(uint32_t id) const
{
    return m_slots[id].current_state;
}

template <typename T>
void ResourcePool<T>::update_current_state(uint32_t id, typename ResourcePool<T>::StateType new_state)
{
    m_slots[id].current_state = new_state;
}

} // namespace orendergraph

#endif // ORENDERGRAPH_RESOURCE_POOL_TPP
