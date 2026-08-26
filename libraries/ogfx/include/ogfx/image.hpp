#ifndef OGFX_IMAGE_HPP
#define OGFX_IMAGE_HPP

#include <ogfx/format.hpp>
#include <ogfx/image_usage.hpp>

#include <cstdint>
#include <memory>

namespace ogfx
{

class Device;

struct ImageDesc
{
    uint32_t width = 0;
    uint32_t height = 0;

    uint32_t mip_levels = 1;
    uint32_t array_layers = 1;

    Format format = Format::Undefined;

    ImageUsage usage = ImageUsage::None;

    bool owned = true;
};

class Image
{
  public:
    Image(const Device& device, const ImageDesc& desc);
    ~Image();

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    Image(Image&&) noexcept;
    Image& operator=(Image&&) noexcept;

    [[nodiscard]] uint32_t width() const;
    [[nodiscard]] uint32_t height() const;
    [[nodiscard]] Format format() const;

  private:
    friend class Swapchain;
    friend class ImageView;

    ImageDesc m_desc;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_IMAGE_HPP