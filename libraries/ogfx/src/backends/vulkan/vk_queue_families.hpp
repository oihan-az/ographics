#ifndef OGFX_VK_QUEUE_FAMILIES_HPP
#define OGFX_VK_QUEUE_FAMILIES_HPP

#include <optional>

namespace ogfx
{

struct QueueFamilyIndices
{
    std::optional<uint32_t> transfer_family;
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> compute_family;
    std::optional<uint32_t> present_family;
};

} // namespace ogfx

#endif // OGFX_VK_QUEUE_FAMILIES_HPP