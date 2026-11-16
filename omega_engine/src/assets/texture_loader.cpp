#include <omega/assets/texture_loader.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <array>

namespace omega
{

namespace
{

enum class PixelType
{
    UInt8,
    UInt16,
    Float32
};

bool get_image_format(int channels, PixelType pixel_type, ogfx::Format& format)
{
    switch (pixel_type)
    {
    case PixelType::UInt8:
        switch (channels)
        {
        case 1:
            format = ogfx::Format::R8_UNORM;
            return true;
        case 2:
            format = ogfx::Format::RG8_UNORM;
            return true;
        case 3:
            format = ogfx::Format::RGB8_UNORM;
            return true;
        case 4:
            format = ogfx::Format::RGBA8_UNORM;
            return true;
        }
        break;

    case PixelType::UInt16:
        switch (channels)
        {
        case 1:
            format = ogfx::Format::R16_UNORM;
            return true;
        case 2:
            format = ogfx::Format::RG16_UNORM;
            return true;
        case 3:
            format = ogfx::Format::RGB16_UNORM;
            return true;
        case 4:
            format = ogfx::Format::RGBA16_UNORM;
            return true;
        }
        break;

    case PixelType::Float32:
        switch (channels)
        {
        case 1:
            format = ogfx::Format::R32_FLOAT;
            return true;
        case 2:
            format = ogfx::Format::RG32_FLOAT;
            return true;
        case 3:
            format = ogfx::Format::RGB32_FLOAT;
            return true;
        case 4:
            format = ogfx::Format::RGBA32_FLOAT;
            return true;
        }
        break;
    }

    return false;
}

bool create_texture_asset(void* pixels, int width, int height, int channels, PixelType pixel_type,
                          TextureAsset& texture)
{
    if (!pixels || width <= 0 || height <= 0)
        return false;

    const bool expand_rgb = channels == 3;
    const uint32_t output_channels = expand_rgb ? 4 : channels;

    TextureAsset result;
    if (!get_image_format(output_channels, pixel_type, result.format))
        return false;

    ogfx::FormatInfo format_info{};
    if (!ogfx::get_format_info(result.format, format_info) || format_info.bytes_per_pixel == 0)
    {
        return false;
    }

    const uint64_t pixel_count = static_cast<uint64_t>(width) * static_cast<uint64_t>(height);
    if (pixel_count > std::numeric_limits<size_t>::max() / format_info.bytes_per_pixel)
    {
        return false;
    }

    const size_t data_size = static_cast<size_t>(pixel_count * format_info.bytes_per_pixel);

    TextureSubresource subresource;
    subresource.data.resize(data_size);

    if (!expand_rgb)
    {
        std::memcpy(subresource.data.data(), pixels, data_size);
    }
    else
    {
        std::array<std::byte, sizeof(float)> alpha_data{};
        size_t alpha_size = 0;

        switch (pixel_type)
        {
        case PixelType::UInt8:
        {
            const uint8_t alpha = UINT8_MAX;
            std::memcpy(alpha_data.data(), &alpha, sizeof(alpha));
            alpha_size = sizeof(alpha);
            break;
        }
        case PixelType::UInt16:
        {
            const uint16_t alpha = UINT16_MAX;
            std::memcpy(alpha_data.data(), &alpha, sizeof(alpha));
            alpha_size = sizeof(alpha);
            break;
        }
        case PixelType::Float32:
        {
            const float alpha = 1.0f;
            std::memcpy(alpha_data.data(), &alpha, sizeof(alpha));
            alpha_size = sizeof(alpha);
            break;
        }
        }

        const size_t input_pixel_size = 3 * (format_info.bytes_per_pixel / 4);
        const size_t output_pixel_size = format_info.bytes_per_pixel;

        const auto* source = static_cast<const std::byte*>(pixels);
        for (size_t i = 0; i < pixel_count; ++i)
        {
            const size_t source_offset = i * input_pixel_size;
            const size_t destination_offset = i * output_pixel_size;

            std::memcpy(subresource.data.data() + destination_offset, source + source_offset, input_pixel_size);
            std::memcpy(subresource.data.data() + destination_offset + input_pixel_size, alpha_data.data(), alpha_size);
        }
    }

    result.type = ogfx::ImageType::Image2D;
    result.width = static_cast<uint32_t>(width);
    result.height = static_cast<uint32_t>(height);
    result.depth = 1;
    result.mip_levels = 1;
    result.array_layers = 1;

    result.subresources.push_back(std::move(subresource));

    texture = std::move(result);
    return true;
}

} // namespace

bool TextureLoader::load(const std::filesystem::path& path, TextureAsset& texture)
{
    const std::string filename = path.string();

    int width = 0;
    int height = 0;
    int channels = 0;

    PixelType pixel_type = PixelType::UInt8;
    void* pixels = nullptr;

    if (stbi_is_hdr(filename.c_str()))
    {
        pixel_type = PixelType::Float32;
        pixels = stbi_loadf(filename.c_str(), &width, &height, &channels, STBI_default);
    }

    if (!pixels && stbi_is_16_bit(filename.c_str()))
    {
        pixel_type = PixelType::UInt16;
        pixels = stbi_load_16(filename.c_str(), &width, &height, &channels, STBI_default);
    }

    if (!pixels)
    {
        pixel_type = PixelType::UInt8;
        pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_default);
    }

    if (!pixels)
        return false;

    const bool success = create_texture_asset(pixels, width, height, channels, pixel_type, texture);

    stbi_image_free(pixels);

    return success;
}

bool TextureLoader::load(std::span<const std::byte> data, TextureAsset& texture)
{
    if (data.empty() || data.size() > static_cast<size_t>(std::numeric_limits<int>::max()))
    {
        return false;
    }

    const auto* encoded_data = reinterpret_cast<const stbi_uc*>(data.data());

    const int data_size = static_cast<int>(data.size());

    int width = 0;
    int height = 0;
    int channels = 0;

    PixelType pixel_type = PixelType::UInt8;
    void* pixels = nullptr;

    if (stbi_is_hdr_from_memory(encoded_data, data_size))
    {
        pixel_type = PixelType::Float32;
        pixels = stbi_loadf_from_memory(encoded_data, data_size, &width, &height, &channels, STBI_default);
    }

    if (!pixels && stbi_is_16_bit_from_memory(encoded_data, data_size))
    {
        pixel_type = PixelType::UInt16;
        pixels = stbi_load_16_from_memory(encoded_data, data_size, &width, &height, &channels, STBI_default);
    }

    if (!pixels)
    {
        pixel_type = PixelType::UInt8;
        pixels = stbi_load_from_memory(encoded_data, data_size, &width, &height, &channels, STBI_default);
    }

    if (!pixels)
        return false;

    const bool success = create_texture_asset(pixels, width, height, channels, pixel_type, texture);

    stbi_image_free(pixels);

    return success;
}

} // namespace omega