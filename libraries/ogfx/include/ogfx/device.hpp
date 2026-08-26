#ifndef OGFX_DEVICE_HPP
#define OGFX_DEVICE_HPP

#include <memory>

namespace ogfx
{

class PhysicalDevice;
class Surface;

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

  private:
    friend class Swapchain;
    friend class Image;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_DEVICE_HPP