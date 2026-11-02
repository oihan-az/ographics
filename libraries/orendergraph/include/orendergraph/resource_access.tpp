#ifndef ORENDERGRAPH_RESOURCE_ACCESS_TPP
#define ORENDERGRAPH_RESOURCE_ACCESS_TPP

namespace orendergraph
{

template <typename T>
ResourceAccess<T>::ResourceAccess(ResourceHandle<T> handle, StateType requested_state, DataType data)
    : m_handle(handle), m_requested_state(requested_state), m_data(data)
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