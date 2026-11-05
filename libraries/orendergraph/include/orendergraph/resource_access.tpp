#ifndef ORENDERGRAPH_RESOURCE_ACCESS_TPP
#define ORENDERGRAPH_RESOURCE_ACCESS_TPP

namespace orendergraph
{

template <typename T>
ResourceAccess<T>::ResourceAccess(ResourceHandle<T> handle, StateType requested_state, DataType data, 
                                  StateType final_state, bool update_state)
    : m_handle(handle), m_requested_state(requested_state), m_data(data), 
      m_final_state(final_state), m_update_state(update_state)
{
}

template <typename T> 
uint32_t ResourceAccess<T>::resource_id() const
{
    return m_handle.id;
}

template <typename T> 
std::type_index ResourceAccess<T>::resource_type_idx() const
{
    return std::type_index(typeid(T));
}

template <typename T> 
void ResourceAccess<T>::update_state(ResourcePoolRegistry& registry)
{
    if (!m_update_state)
        return;

    auto* pool = registry.get_pool<T>();
    pool->update_current_state(m_handle.id, m_final_state);
}

template <typename T> 
void ResourceAccess<T>::transition(ogfx::CommandBuffer& cmd, ResourcePoolRegistry& registry)
{
    T* physical_resource = registry.get_resource<T>(m_handle);
    if (physical_resource == nullptr)
        return;

    auto* pool = registry.get_pool<T>();

    StateType old_state = pool->get_current_state(m_handle.id);
    Traits::transition(cmd, physical_resource, old_state, m_requested_state, m_data);
    pool->update_current_state(m_handle.id, m_requested_state);
}

} // namespace orendergraph

#endif // ORENDERGRAPH_RESOURCE_ACCESS_TPP