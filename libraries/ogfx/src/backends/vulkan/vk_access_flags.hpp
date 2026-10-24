#ifndef OGFX_VK_ACCESS_FLAGS_HPP
#define OGFX_VK_ACCESS_FLAGS_HPP

#include <ogfx/access_flags.hpp>
#include <vulkan/vulkan.h>

namespace ogfx
{

VkAccessFlags to_vk_access_flags(AccessFlags flags);
AccessFlags from_vk_access_flags(VkAccessFlags flags);

} // namespace ogfx

#endif // OGFX_VK_ACCESS_FLAGS_HPP