#ifndef OGFX_BUFFER_HPP
#define OGFX_BUFFER_HPP

#include <ogfx/buffer_usage.hpp>
#include <ogfx/memory_property.hpp>

#include <cstdint>
#include <memory>

namespace ogfx
{

class PhysicalDevice;
class Device;

struct BufferWriteInfo
{
    const void* data = nullptr;
    uint64_t size = 0;
    uint64_t offset = 0;
};

struct BufferDesc
{
    uint64_t size = 0;
    BufferUsage usage = BufferUsage::None;
    MemoryProperty memory_properties = MemoryProperty::DeviceLocal;
};

class Buffer
{
  public:
    Buffer(const PhysicalDevice& physical_device, const Device& device, const BufferDesc& desc);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&&) noexcept;
    Buffer& operator=(Buffer&&) noexcept;

    void allocate_memory();
    void free_memory();
    void bind_memory();

    void write(const BufferWriteInfo& info);

  private:
    friend class CommandBuffer;
    friend class ResourceSet;

    void create();
    void destroy();

    BufferDesc m_desc{};

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_BUFFER_HPP