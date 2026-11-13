#ifndef OMEGA_TEXTURE_ASSET_HPP
#define OMEGA_TEXTURE_ASSET_HPP

#include <ogfx/image.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace omega
{

struct TextureSubresource
{
    uint32_t mip_level = 0;
    uint32_t array_layer = 0;

    std::vector<std::byte> data;
};


struct TextureAsset
{
    ogfx::ImageType type = ogfx::ImageType::Image2D;

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;

    uint32_t mip_levels = 1;
    uint32_t array_layers = 1;

    ogfx::Format format = ogfx::Format::Undefined;

    std::vector<TextureSubresource> subresources;
};

bool validate_texture_asset(const TextureAsset& asset);

} // namespace omega

#endif // OMEGA_TEXTURE_ASSET_HPP