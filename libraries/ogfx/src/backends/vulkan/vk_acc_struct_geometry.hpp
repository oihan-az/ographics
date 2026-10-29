#ifndef OGFX_VK_ACCELERATION_STRUCTURE_GEOMETRY_HPP
#define OGFX_VK_ACCELERATION_STRUCTURE_GEOMETRY_HPP

#include <ogfx/acc_struct_geometry.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkAccelerationStructureGeometryKHR to_vk_acceleration_structure_triangle_geometry(const AccelerationStructureTriangleGeometry& geometry);
VkAccelerationStructureGeometryKHR to_vk_acceleration_structure_instance_geometry(const AccelerationStructureInstanceGeometry& geometry);

} // namespace ogfx

#endif // OGFX_VK_ACCELERATION_STRUCTURE_GEOMETRY_HPP