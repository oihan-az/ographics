#ifndef OGFX_VK_RASTERIZATION_HPP
#define OGFX_VK_RASTERIZATION_HPP

#include <ogfx/rasterization.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkPolygonMode to_vk_polygon_mode(PolygonMode mode);
PolygonMode from_vk_polygon_mode(VkPolygonMode mode);

VkCullModeFlags to_vk_cull_mode(CullMode mode);
CullMode from_vk_cull_mode(VkCullModeFlags mode);

VkFrontFace to_vk_front_face(FrontFace face);
FrontFace from_vk_front_face(VkFrontFace face);

} // namespace ogfx

#endif // OGFX_VK_RASTERIZATION_HPP