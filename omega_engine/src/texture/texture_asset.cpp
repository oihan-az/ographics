#include <omega/texture/texture_asset.hpp>

#include <unordered_set>

namespace
{

uint32_t mip_dimension(uint32_t dimension, uint32_t mip_level)
{
    return std::max(1u, dimension >> mip_level);
}

bool multiply_overflow(uint64_t a, uint64_t b, uint64_t& result)
{
    if (a != 0 && b > std::numeric_limits<uint64_t>::max() / a)
        return true;

    result = a * b;
    return false;
}

bool calculate_subresource_size(uint32_t width, uint32_t height, uint32_t depth, uint32_t bytes_per_pixel,
                                uint64_t& out_size)
{
    uint64_t size = width;

    if (multiply_overflow(size, height, size))
        return false;

    if (multiply_overflow(size, depth, size))
        return false;

    if (multiply_overflow(size, bytes_per_pixel, size))
        return false;

    out_size = size;
    return true;
}

} // namespace

namespace omega
{

bool validate_texture_asset(const TextureAsset& asset)
{
    if (asset.width == 0 || asset.height == 0 || asset.depth == 0)
        return false;

    if (asset.mip_levels == 0 || asset.array_layers == 0)
        return false;

    ogfx::FormatInfo format_info{};

    if (!ogfx::get_format_info(asset.format, format_info))
        return false;

    if (format_info.bytes_per_pixel == 0)
        return false;

    switch (asset.type)
    {
    case ogfx::ImageType::Image1D:
        if (asset.height != 1 || asset.depth != 1)
            return false;
        break;

    case ogfx::ImageType::Image2D:
        if (asset.depth != 1)
            return false;
        break;

    case ogfx::ImageType::Image3D:
        if (asset.array_layers != 1)
            return false;
        break;

    case ogfx::ImageType::Cube:
        if (asset.depth != 1 || asset.width != asset.height)
            return false;

        if (asset.array_layers % 6 != 0)
            return false;
        break;

    case ogfx::ImageType::Undefined:
    default:
        return false;
    }

    uint32_t max_dimension = asset.width;

    if (asset.type != ogfx::ImageType::Image1D)
        max_dimension = std::max(max_dimension, asset.height);

    if (asset.type == ogfx::ImageType::Image3D)
        max_dimension = std::max(max_dimension, asset.depth);

    uint32_t max_mip_levels = 1;

    while (max_dimension > 1)
    {
        max_dimension >>= 1;
        ++max_mip_levels;
    }

    if (asset.mip_levels > max_mip_levels)
        return false;

    const uint64_t expected_subresource_count = static_cast<uint64_t>(asset.mip_levels) * (asset.type == ogfx::ImageType::Image3D ? 1 : asset.array_layers);

    if (asset.subresources.size() != expected_subresource_count)
        return false;

    std::unordered_set<uint64_t> seen_subresources;
    seen_subresources.reserve(asset.subresources.size());

    for (const TextureSubresource& subresource : asset.subresources)
    {
        if (subresource.mip_level >= asset.mip_levels)
            return false;

        const uint32_t layer_count = asset.type == ogfx::ImageType::Image3D ? 1 : asset.array_layers;
        if (subresource.array_layer >= layer_count)
            return false;

        const uint64_t key = static_cast<uint64_t>(subresource.mip_level) * layer_count + subresource.array_layer;

        if (!seen_subresources.insert(key).second)
            return false;

        const uint32_t mip_width = mip_dimension(asset.width, subresource.mip_level);
        const uint32_t mip_height = mip_dimension(asset.height, subresource.mip_level);
        const uint32_t mip_depth = asset.type == ogfx::ImageType::Image3D ? mip_dimension(asset.depth, subresource.mip_level) : 1;

        uint64_t expected_size = 0;
        if (!calculate_subresource_size(mip_width, mip_height, mip_depth, format_info.bytes_per_pixel, expected_size))
        {
            return false;
        }

        if (subresource.data.size() != expected_size)
            return false;
    }

    return true;
}
} // namespace omega