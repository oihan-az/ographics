#ifndef OGFX_DEVICE_HPP
#define OGFX_DEVICE_HPP

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

struct DeviceDesc
{
    bool enable_transfer = true;
    bool enable_graphics = true;
    bool enable_compute = true;
    bool enable_present = true;
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

    std::array<Queue, to_index(QueueType::Count)> m_queues;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_DEVICE_HPP