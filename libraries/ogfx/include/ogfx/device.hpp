#ifndef OGFX_DEVICE_HPP
#define OGFX_DEVICE_HPP

#include <memory>

namespace ogfx
{

class PhysicalDevice;
class Queue;

struct DeviceDesc
{
    bool enable_transfer = true;
    bool enable_graphics = true;
    bool enable_compute = true;
};

class Device
{
  public:
    explicit Device(const PhysicalDevice& physical_device, const DeviceDesc& desc);
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    Device(Device&&) noexcept;
    Device& operator=(Device&&) noexcept;

  private:
    friend class Queue;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_DEVICE_HPP