#ifndef OGFX_PHYSICAL_DEVICE_HPP
#define OGFX_PHYSICAL_DEVICE_HPP

#include <cstdint>
#include <memory>
#include <string>

namespace ogfx
{

struct PhysicalDeviceProperties
{
    std::string name;

    uint32_t vendor_id = 0;
    uint32_t device_id = 0;

    uint32_t api_version = 0;
    uint32_t driver_version = 0;
};

class PhysicalDevice
{
  private:
    struct Impl;

  public:
    explicit PhysicalDevice(std::unique_ptr<Impl> impl);
    ~PhysicalDevice();

    PhysicalDevice(const PhysicalDevice&) = delete;
    PhysicalDevice& operator=(const PhysicalDevice&) = delete;

    PhysicalDevice(PhysicalDevice&&) noexcept;
    PhysicalDevice& operator=(PhysicalDevice&&) noexcept;

    const PhysicalDeviceProperties& properties() const;

  private:
    friend class Instance;
    friend class Device;

    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_PHYSICAL_DEVICE_HPP