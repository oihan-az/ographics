#ifndef OGFX_IMAGE_USAGE_HPP
#define OGFX_IMAGE_USAGE_HPP

#include <cstdint>
#include <string>

namespace ogfx
{

enum class ImageUsage : uint32_t
{
    None = 0,
    TransferSrc = 1 << 0,
    TransferDst = 1 << 1,
    Sampled = 1 << 2,
    Storage = 1 << 3,
    ColorAttachment = 1 << 4,
    DepthStencil = 1 << 5
};

constexpr ImageUsage operator|(ImageUsage lhs, ImageUsage rhs)
{
    return static_cast<ImageUsage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr bool has_flag(ImageUsage usage, ImageUsage flag)
{
    return (static_cast<uint32_t>(usage) & static_cast<uint32_t>(flag)) != 0;
}

std::string to_string(ImageUsage usage);

} // namespace ogfx

#endif // OGFX_IMAGE_USAGE_HPP