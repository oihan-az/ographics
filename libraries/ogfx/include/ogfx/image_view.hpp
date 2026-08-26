#ifndef OGFX_IMAGE_VIEW_HPP
#define OGFX_IMAGE_VIEW_HPP

#include <cstdint>
#include <memory>

namespace ogfx
{

class Image;

class ImageView
{
  public:
    ImageView(Image& image);
    ~ImageView();

    ImageView(const ImageView&) = delete;
    ImageView& operator=(const ImageView&) = delete;

    ImageView(ImageView&&) noexcept;
    ImageView& operator=(ImageView&&) noexcept;

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_IMAGE_VIEW_HPP