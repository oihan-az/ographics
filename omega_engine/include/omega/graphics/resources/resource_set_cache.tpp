#ifndef ORENDERER_RESOURCE_SET_CACHE_TPP
#define ORENDERER_RESOURCE_SET_CACHE_TPP

#include <ogfx/resource_set_layout.hpp>
#include <ogfx/resource_set.hpp>
#include <ogfx/resource_allocator.hpp>
#include "resource_set_cache.hpp"

namespace omega
{

template <typename Handle, typename Hash>
ResourceSetCache<Handle, Hash>::ResourceSetCache(const ogfx::Device& device, ogfx::ResourceAllocator& allocator, const ogfx::ResourceSetLayout& layout)
    : m_device(device), m_allocator(allocator), m_layout(layout)
{}

template <typename Handle, typename Hash>
bool ResourceSetCache<Handle, Hash>::acquire(Handle handle)
{
    if (!handle.is_valid())
        return false;

    auto it = m_entries.find(handle);
    if (it != m_entries.end())
    {
        ++it->second.reference_count;
        return true;
    }

    Entry entry;
    entry.reference_count = 1;

    try
    {
        ogfx::ResourceSetDesc desc{};
        desc.layout = &m_layout;

        entry.resource_set = std::make_unique<ogfx::ResourceSet>(m_device, desc);
        entry.resource_set->allocate(m_allocator);
    }
    catch (...)
    {
        return false;
    }

    m_entries.emplace(handle, std::move(entry));
    return true;
}

template <typename Handle, typename Hash>
bool ResourceSetCache<Handle, Hash>::release(Handle handle)
{
    auto it = m_entries.find(handle);
    if (it == m_entries.end())
        return false;

    if (--it->second.reference_count == 0)
    {
        m_entries.erase(it);
        return true;
    }

    return false;
}

template <typename Handle, typename Hash>
bool ResourceSetCache<Handle, Hash>::update(const Handle& handle, const ogfx::ResourceSetWriteInfo& write_info)
{
    auto it = m_entries.find(handle);
    if (it == m_entries.end())
        return false;

    it->second.resource_set->write(write_info);
    return true;
}

template <typename Handle, typename Hash>
ogfx::ResourceSet* ResourceSetCache<Handle, Hash>::get(Handle handle)
{
    auto it = m_entries.find(handle);
    if (it == m_entries.end())
        return nullptr;

    return it->second.resource_set.get();
}

template <typename Handle, typename Hash>
const ogfx::ResourceSet* ResourceSetCache<Handle, Hash>::get(Handle handle) const
{
    auto it = m_entries.find(handle);
    if (it == m_entries.end())
        return nullptr;

    return it->second.resource_set.get();
}

template <typename Handle, typename Hash>
bool ResourceSetCache<Handle, Hash>::contains(Handle handle) const
{
    return m_entries.contains(handle);
}

template <typename Handle, typename Hash>
uint32_t ResourceSetCache<Handle, Hash>::reference_count(Handle handle) const
{
    const auto it = m_entries.find(handle);
    return it != m_entries.end() ? it->second.reference_count : 0;
}

template <typename Handle, typename Hash>
uint32_t ResourceSetCache<Handle, Hash>::size() const
{
    return static_cast<uint32_t>(m_entries.size());
}

template <typename Handle, typename Hash> 
void ResourceSetCache<Handle, Hash>::clear()
{
    m_entries.clear();
}

} // namespace omega

#endif // ORENDERER_RESOURCE_SET_CACHE_TPP