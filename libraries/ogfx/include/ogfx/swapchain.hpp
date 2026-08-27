#ifndef OGFX_SWAPCHAIN_HPP
#define OGFX_SWAPCHAIN_HPP

#include <ogfx/format.hpp>
#include <ogfx/present_mode.hpp>

#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>

#include <cstdint>
#include <memory>
#include <vector>
#include <stdexcept>

namespace ogfx
{

class PhysicalDevice;
class Device;
class Surface;
class Semaphore;

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

    uint32_t acquire_next_image(Semaphore& signal_semaphore);

    [[nodiscard]] uint32_t image_count() const
    {
        return static_cast<uint32_t>(m_image_views.size());
    }

    [[nodiscard]] ImageView& image_view(uint32_t index)
    {
        if (index >= m_image_views.size())
        {
            throw std::out_of_range("Swapchain image view index out of range.");
        }

        return m_image_views[index];
    }

    [[nodiscard]] const ImageView& image_view(uint32_t index) const
    {
        if (index >= m_image_views.size())
        {
            throw std::out_of_range("Swapchain image view index out of range.");
        }

        return m_image_views[index];
    }

  private:

    friend class Queue;

    std::vector<Image> m_images;
    std::vector<ImageView> m_image_views;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_SWAPCHAIN_HPP