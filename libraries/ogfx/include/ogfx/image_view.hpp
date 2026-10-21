#ifndef OGFX_IMAGE_VIEW_HPP
#define OGFX_IMAGE_VIEW_HPP

#include <cstdint>
#include <memory>

namespace ogfx
{

class Image;

enum class ImageViewType
{
    Undefined,
    View1D,
    View2D,
    View3D,
    View1DArray,
    View2DArray,
    Cube,
    CubeArray
};

struct ImageViewDesc
{
    const Image* image = nullptr;

    ImageViewType type = ImageViewType::View2D;

    uint32_t base_mip_level = 0;
    uint32_t mip_level_count = 1;

    uint32_t base_array_layer = 0;
    uint32_t array_layer_count = 1;
};

class ImageView
{
  public:
    ImageView(const ImageViewDesc& desc);
    ~ImageView();

    ImageView(const ImageView&) = delete;
    ImageView& operator=(const ImageView&) = delete;

    ImageView(ImageView&&) noexcept;
    ImageView& operator=(ImageView&&) noexcept;

  private:
    friend class Framebuffer;
    friend class ResourceSet;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_IMAGE_VIEW_HPP