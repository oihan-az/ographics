#ifndef OGFX_RESOURCE_ALLOCATOR_HPP
#define OGFX_RESOURCE_ALLOCATOR_HPP

#include <ogfx/resource_type.hpp>

#include <cstdint>
#include <memory>

namespace ogfx
{

class Device;

struct DescriptorPoolSize
{
    ResourceType type = ResourceType::Undefined;
    uint32_t descriptor_count = 0;
};

struct DescriptorPoolDesc
{
    uint32_t max_sets = 1;
    std::vector<DescriptorPoolSize> sizes;
};

class ResourceAllocator
{
  public:
    ResourceAllocator(const Device& device);
    ~ResourceAllocator();

    ResourceAllocator(const ResourceAllocator&) = delete;
    ResourceAllocator& operator=(const ResourceAllocator&) = delete;

    ResourceAllocator(ResourceAllocator&&) noexcept;
    ResourceAllocator& operator=(ResourceAllocator&&) noexcept;

    void create_pool(const DescriptorPoolDesc& desc);

  private:
    friend class ResourceSet;

    void allocate(ResourceSet& resource_set);
    void free(ResourceSet& resource_set);

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_RESOURCE_ALLOCATOR_HPP
