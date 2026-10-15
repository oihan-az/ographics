#ifndef OGFX_VK_INPUT_ASSEMBLY_HPP
#define OGFX_VK_INPUT_ASSEMBLY_HPP

#include <ogfx/input_assembly.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkPrimitiveTopology to_vk_primitive_topology(PrimitiveTopology topology);
PrimitiveTopology from_vk_primitive_topology(VkPrimitiveTopology topology);

} // namespace ogfx

#endif // OGFX_VK_INPUT_ASSEMBLY_HPP