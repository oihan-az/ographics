#ifndef OGFX_RESOURCE_SET_HPP
#define OGFX_RESOURCE_SET_HPP

#include <cstdint>
#include <memory>
#include <vector>

namespace ogfx
{

class Device;
class ResourceSetLayout;
class ResourceAllocator;
class Buffer;

struct BufferBinding
{
    uint32_t binding = 0;
    const Buffer* buffer = nullptr;
    uint64_t offset = 0;
    uint64_t size = 0;
};

struct ResourceSetDesc
{
    const ResourceSetLayout* layout = nullptr;
};

struct ResourceSetWriteInfo
{
    std::vector<BufferBinding> buffers;
};

class ResourceSet
{
  public:
    ResourceSet(const Device& device, const ResourceSetDesc& desc);
    ~ResourceSet();

    ResourceSet(const ResourceSet&) = delete;
    ResourceSet& operator=(const ResourceSet&) = delete;

    ResourceSet(ResourceSet&&) noexcept;
    ResourceSet& operator=(ResourceSet&&) noexcept;

    void allocate(ResourceAllocator& allocator);
    void free(ResourceAllocator& allocator);

    void write(const ResourceSetWriteInfo& info);

  private:
    friend class CommandBuffer;
    friend class ResourceAllocator;

    ResourceSetDesc m_desc{};

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_RESOURCE_SET_HPP
