#include "vk_acc_struct_geometry.hpp"

#include "vk_format.hpp"
#include "vk_index_type.hpp"

#include <ogfx/buffer.hpp>

namespace ogfx
{

VkAccelerationStructureGeometryKHR
to_vk_acceleration_structure_triangle_geometry(const AccelerationStructureTriangleGeometry& geometry)
{
    VkAccelerationStructureGeometryKHR vk_geometry{};
    vk_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    vk_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    vk_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

    VkAccelerationStructureGeometryTrianglesDataKHR triangles{};
    triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    triangles.vertexFormat = to_vk_format(geometry.vertex_format);
    triangles.vertexData.deviceAddress = geometry.vertex_buffer ? geometry.vertex_buffer->device_address() + geometry.vertex_offset : 0;
    triangles.vertexStride = geometry.vertex_stride;
    triangles.maxVertex = geometry.vertex_count > 0 ? geometry.vertex_count - 1 : 0;
    triangles.indexType = geometry.index_buffer ? to_vk_index_type(geometry.index_type) : VK_INDEX_TYPE_NONE_KHR;
    triangles.indexData.deviceAddress = geometry.index_buffer ? geometry.index_buffer->device_address() + geometry.index_offset : 0;

    vk_geometry.geometry.triangles = triangles;

    return vk_geometry;
}

VkAccelerationStructureGeometryKHR
to_vk_acceleration_structure_instance_geometry(const AccelerationStructureInstanceGeometry& geometry)
{
    VkAccelerationStructureGeometryKHR vk_geometry{};
    vk_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    vk_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    vk_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

    VkAccelerationStructureGeometryInstancesDataKHR instances{};
    instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    instances.arrayOfPointers = VK_FALSE;
    instances.data.deviceAddress = geometry.instance_buffer ? geometry.instance_buffer->device_address() + geometry.instance_offset : 0;

    vk_geometry.geometry.instances = instances;

    return vk_geometry;
}

} // namespace ogfx