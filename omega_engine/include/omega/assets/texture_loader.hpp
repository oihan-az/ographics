#ifndef OMEGA_TEXTURE_LOADER_HPP
#define OMEGA_TEXTURE_LOADER_HPP

#include <omega/texture/texture_asset.hpp>

#include <filesystem>
#include <cstddef>
#include <span>

namespace omega
{

class TextureLoader
{
  public:
    static bool load(const std::filesystem::path& path, TextureAsset& texture);
    static bool load(uint32_t width, uint32_t height, std::span<const std::byte> rgba_data, TextureAsset& texture);
    static bool load(std::span<const std::byte> data, TextureAsset& texture);
};

} // namespace omega

#endif // OMEGA_TEXTURE_LOADER_HPP