#ifndef OGFX_IMAGE_HPP
#define OGFX_IMAGE_HPP

#include <ogfx/format.hpp>
#include <ogfx/image_usage.hpp>

#include <cstdint>
#include <memory>

namespace ogfx
{

class PhysicalDevice;
class Device;

enum class ImageType
{
    Undefined,
    Image1D,
    Image2D,
    Image3D,
    Cube
};

struct ImageWriteInfo
{
    const void* data = nullptr;
    uint64_t size = 0;
};

struct ImageDesc
{
    ImageType type = ImageType::Image2D;

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
    Image(const PhysicalDevice& physical_device, const Device& device, const ImageDesc& desc);
    ~Image();

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    Image(Image&&) noexcept;
    Image& operator=(Image&&) noexcept;

    void allocate_memory();
    void free_memory();
    void bind_memory();

    [[nodiscard]] uint32_t width() const { return m_desc.width; }
    [[nodiscard]] uint32_t height() const { return m_desc.height; }
    [[nodiscard]] Format format() const { return m_desc.format; }

  private:
    friend class Swapchain;
    friend class ImageView;
    friend class CommandBuffer;

    ImageDesc m_desc;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_IMAGE_HPP