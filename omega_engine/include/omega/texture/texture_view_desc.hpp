#ifndef OMEGA_TEXTURE_VIEW_DESC_HPP
#define OMEGA_TEXTURE_VIEW_DESC_HPP

#include <ogfx/image_view.hpp>

namespace omega
{

struct TextureViewDesc
{
    ogfx::ImageViewType type = ogfx::ImageViewType::View2D;
    ogfx::ImageAspect aspect = ogfx::ImageAspect::Color;

    uint32_t base_mip_level = 0;
    uint32_t mip_level_count = 1;

    uint32_t base_array_layer = 0;
    uint32_t array_layer_count = 1;
};

} // namespace omega

#endif // OMEGA_TEXTURE_VIEW_DESC_HPP