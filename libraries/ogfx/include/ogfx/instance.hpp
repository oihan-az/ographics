#ifndef OGFX_INSTANCE_HPP
#define OGFX_INSTANCE_HPP

#include <ogfx/physical_device.hpp>

#include <memory>
#include <vector>

namespace ogfx
{

struct InstanceDesc
{
    const char* application_name = "ogfx";
    const char* engine_name = "ogfx";

    bool debug = false;
};

class Instance
{
  public:
    explicit Instance(const InstanceDesc& desc);
    ~Instance();

    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;

    Instance(Instance&&) noexcept;
    Instance& operator=(Instance&&) noexcept;

    [[nodiscard]] std::vector<PhysicalDevice> enumerate_physical_devices() const;
    [[nodiscard]] PhysicalDevice pick_physical_device(const std::vector<PhysicalDevice>& physicalDevice) const;

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_INSTANCE_HPP