#ifndef OGFX_DEVICE_HPP
#define OGFX_DEVICE_HPP

#include <ogfx/enum_flags.hpp>
#include <ogfx/queue.hpp>

#include <memory>
#include <array>

namespace ogfx
{

class PhysicalDevice;
class Surface;

enum class QueueType
{
    Transfer,
    Graphics,
    Compute,
    Present,
    Count
};

constexpr size_t to_index(QueueType type)
{
    return static_cast<size_t>(type);
}

OGFX_BITMASK_ENUM(DeviceQueue, uint32_t,
    None     = 0,
    Transfer = 1 << 0,
    Graphics = 1 << 1,
    Compute  = 1 << 2,
    Present  = 1 << 3
);

OGFX_BITMASK_ENUM(DeviceFeature, uint32_t,
    None                   = 0,
    MultiDrawIndirect       = 1 << 0,
    DrawIndirectFirstInstance = 1 << 1,
    SamplerAnisotropy       = 1 << 2
);

struct DeviceDesc
{
    DeviceQueue queues = DeviceQueue::Transfer | DeviceQueue::Graphics | DeviceQueue::Compute | DeviceQueue::Present;
    DeviceFeature features = DeviceFeature::None;
};

class Device
{
  public:
    explicit Device(const PhysicalDevice& physical_device, const DeviceDesc& desc,
                    const Surface* surface = nullptr);
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    Device(Device&&) noexcept;
    Device& operator=(Device&&) noexcept;

    void wait_idle();

    [[nodiscard]] Queue& queue(QueueType type)
    {
        return m_queues[to_index(type)];
    }

    [[nodiscard]] const Queue& queue(QueueType type) const
    {
        return m_queues[to_index(type)];
    }

  private:
    friend class Swapchain;
    friend class Image;
    friend class CommandPool;
    friend class Semaphore;
    friend class Fence;
    friend class RenderPass;
    friend class Framebuffer;
    friend class Shader;
    friend class GraphicsPipeline;
    friend class ComputePipeline;
    friend class Buffer;
    friend class ResourceSetLayout;
    friend class ResourceAllocator;
    friend class PipelineLayout;
    friend class Sampler;

    std::array<Queue, to_index(QueueType::Count)> m_queues;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_DEVICE_HPP