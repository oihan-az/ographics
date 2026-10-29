#ifndef OGFX_VK_ACCELERATION_STRUCTURE_TYPE_HPP
#define OGFX_VK_ACCELERATION_STRUCTURE_TYPE_HPP

#include <ogfx/acc_struct_type.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkAccelerationStructureTypeKHR to_vk_acceleration_structure_type(AccelerationStructureType type);
AccelerationStructureType from_vk_acceleration_structure_type(VkAccelerationStructureTypeKHR type);

} // namespace ogfx

#endif // OGFX_VK_ACCELERATION_STRUCTURE_TYPE_HPP