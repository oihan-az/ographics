#ifndef ORENDERGRAPH_RESOURCE_POOL_REGISTRY_TPP
#define ORENDERGRAPH_RESOURCE_POOL_REGISTRY_TPP

namespace orendergraph
{

template <typename T> 
ResourcePool<T>* ResourcePoolRegistry::get_pool()
{
    const auto type_idx = std::type_index(typeid(T));
    auto it = m_pools.find(type_idx);

    if (it == m_pools.end())
    {
        auto new_pool = std::make_unique<ResourcePool<T>>();
        auto* pool_ptr = new_pool.get();

        m_pools.emplace(type_idx, std::move(new_pool));

        return pool_ptr;
    }

    return static_cast<ResourcePool<T>*>(it->second.get());
}

template <typename T> 
T* ResourcePoolRegistry::get_resource(ResourceHandle<T> handle)
{
    if (!handle.is_valid())
        return nullptr;

    auto* pool = get_pool<T>();

    if (handle.id >= pool->slot_count())
        return nullptr;

    return pool->slot(handle.id).physical_resource;
}

template <typename T>
ResourceHandle<T> ResourcePoolRegistry::create_resource(const ogfx::PhysicalDevice& physical_device,
                                                        const ogfx::Device& device,
                                                        typename ResourceTraits<T>::DescType& desc)
{
    auto* pool = get_pool<T>();
    auto* resource = ResourceTraits<T>::create(physical_device, device, desc);
    ResourceHandle<T> handle = pool->add_slot(resource, false);

    ResourceTraits<T>::allocate(pool->slot(handle.id).physical_resource);

    return handle;
}

template <typename T> 
bool ResourcePoolRegistry::release_resource(ResourceHandle<T> handle)
{
    if (!handle.is_valid())
        return false;

    const auto type_idx = std::type_index(typeid(T));
    auto it = m_pools.find(type_idx);
    if (it == m_pools.end())
        return false;

    auto* pool = static_cast<ResourcePool<T>*>(it->second.get());
    return pool->remove_slot(handle);
}

template <typename T> 
ResourceHandle<T> ResourcePoolRegistry::import_resource(T* resource)
{
    auto* pool = get_pool<T>();
    return pool->add_slot(resource, true);
}

} // namespace orendergraph

#endif