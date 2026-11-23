#ifndef OMEGA_MATERIAL_SYSTEM_TPP
#define OMEGA_MATERIAL_SYSTEM_TPP

#include <utility>

namespace omega
{

template <typename T> 
MaterialPool<T>& MaterialSystem::get_or_create_pool()
{
    const std::type_index type = typeid(T);
    auto it = m_pools.find(type);
    if (it == m_pools.end())
    {
        it = m_pools.emplace(type, std::make_unique<MaterialPool<T>>()).first;
    }

    return *static_cast<MaterialPool<T>*>(it->second.get());
}

template <typename T> 
MaterialPool<T>* MaterialSystem::find_pool()
{
    const auto it = m_pools.find(std::type_index(typeid(T)));
    if (it == m_pools.end())
        return nullptr;

    return static_cast<MaterialPool<T>*>(it->second.get());
}

template <typename T> 
const MaterialPool<T>* MaterialSystem::find_pool() const
{
    const auto it = m_pools.find(std::type_index(typeid(T)));
    if (it == m_pools.end())
        return nullptr;

    return static_cast<const MaterialPool<T>*>(it->second.get());
}

template <typename T> 
MaterialHandle<T> MaterialSystem::create(T material)
{
    return get_or_create_pool<T>().create(std::move(material));
}

template <typename T> 
T* MaterialSystem::get(MaterialHandle<T> handle)
{
    auto* pool = find_pool<T>();
    return pool ? pool->get(handle) : nullptr;
}

template <typename T> 
const T* MaterialSystem::get(MaterialHandle<T> handle) const
{
    const auto* pool = find_pool<T>();
    return pool ? pool->get(handle) : nullptr;
}

template <typename T> 
bool MaterialSystem::has(MaterialHandle<T> handle) const
{
    return get(handle) != nullptr;
}

template <typename T> 
bool MaterialSystem::destroy(MaterialHandle<T> handle)
{
    auto* pool = find_pool<T>();
    return pool ? pool->destroy(handle) : false;
}

} // namespace omega

#endif // OMEGA_MATERIAL_SYSTEM_TPP