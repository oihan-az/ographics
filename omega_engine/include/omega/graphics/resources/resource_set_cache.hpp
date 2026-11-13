#ifndef OMEGA_GRAPHICS_RESOURCES_SET_CACHE_HPP
#define OMEGA_GRAPHICS_RESOURCES_SET_CACHE_HPP

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace ogfx
{

class ResourceAllocator;
class ResourceSetLayout;
class ResourceSet;
class ResourceSetWriteInfo;
class Device;

}

namespace omega
{

template <typename Handle, typename Hash> 
class ResourceSetCache
{
  public:
    ResourceSetCache(const ogfx::Device& device, ogfx::ResourceAllocator& allocator, const ogfx::ResourceSetLayout& layout);

    ~ResourceSetCache() = default;

    bool acquire(Handle handle);
    bool release(Handle handle);
    bool update(const Handle& handle, const ogfx::ResourceSetWriteInfo& write_info);

    ogfx::ResourceSet* get(Handle handle);
    const ogfx::ResourceSet* get(Handle handle) const;

    bool contains(Handle handle) const;

    uint32_t reference_count(Handle handle) const;
    uint32_t size() const;

    void clear();

private:
    struct Entry
    {
        uint32_t reference_count = 0;
        std::unique_ptr<ogfx::ResourceSet> resource_set;
    };

    const ogfx::Device& m_device;
    ogfx::ResourceAllocator& m_allocator;
    const ogfx::ResourceSetLayout& m_layout;

    std::unordered_map<Handle, Entry, Hash> m_entries;
};

} // namespace omega

#include "resource_set_cache.tpp"

#endif // OMEGA_GRAPHICS_RESOURCES_SET_CACHE_HPP