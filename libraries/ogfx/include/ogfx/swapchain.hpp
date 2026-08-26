#ifndef OGFX_SWAPCHAIN_HPP
#define OGFX_SWAPCHAIN_HPP

#include <ogfx/format.hpp>
#include <ogfx/present_mode.hpp>

#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace ogfx
{

class PhysicalDevice;
class Device;
class Surface;

struct SwapchainDesc
{
    uint32_t width = 1280;
    uint32_t height = 720;

    Format format = Format::BGRA8_UNORM;
    PresentMode present_mode = PresentMode::Immediate;

    uint32_t image_count = 3;
};

class Swapchain
{
  public:
    explicit Swapchain(const PhysicalDevice& physical_device, const Device& device, const Surface& surface,
                       const SwapchainDesc& desc);

    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    Swapchain(Swapchain&&) noexcept;
    Swapchain& operator=(Swapchain&&) noexcept;

  private:

    std::vector<Image> m_images;
    std::vector<ImageView> m_image_views;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_SWAPCHAIN_HPP