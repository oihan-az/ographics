#ifndef OMEGA_TEXTURE_DATA_HPP
#define OMEGA_TEXTURE_DATA_HPP

#include <omega/texture/texture_handle.hpp>

#include <ogfx/image.hpp>

#include <cstdint>

namespace omega
{

struct TextureData
{
    TextureHandle handle;
    TextureAsset asset;
};

} // namespace omega

#endif // OMEGA_TEXTURE_DATA_HPP