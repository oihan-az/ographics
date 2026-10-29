#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/acc_struct_commands.hpp>

#include <vulkan/vulkan.h>

#include "../vk_command_buffer.hpp"
#include "../vk_acc_struct.hpp"
#include "../vk_acc_struct_type.hpp"
#include "../vk_acc_struct_geometry.hpp"
#include "../vk_acc_struct_build_preference.hpp"
#include "../vk_device_functions.hpp"
#include "../vk_buffer.hpp"

namespace ogfx
{

void CommandBuffer::build_acceleration_structure(const AccelerationStructureBuildInfo& info)
{
    if (!m_impl || info.destination == nullptr)
    {
        OGFX_LOG_ERROR("Cannot build acceleration structure: invalid destination");
        return;
    }

    if (info.destination->m_impl->m_scratch_buffer == nullptr)
    {
        OGFX_LOG_ERROR("Cannot build acceleration structure: missing scratch buffer");
        return;
    }

    VkAccelerationStructureTypeKHR type = to_vk_acceleration_structure_type(info.destination->type());
    VkBuildAccelerationStructureFlagsKHR flags = info.destination->m_impl->m_build_flags;

    if (info.allow_update)
    {
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    }

    OGFX_LOG("Recording acc. struct. build:");
    OGFX_LOG("  Type: " + to_string_enum(info.destination->type()));
    OGFX_LOG("  Allow update: " + std::string(info.allow_update ? "True" : "False"));
    OGFX_LOG("  Geometries to build: " + std::to_string(info.destination->m_impl->m_primitive_counts.size()));

    std::vector<VkAccelerationStructureBuildRangeInfoKHR> build_ranges;
    build_ranges.reserve(info.destination->m_impl->m_primitive_counts.size());

    size_t geometry_index = 0;
    for (const uint32_t primitive_count : info.destination->m_impl->m_primitive_counts)
    {
        OGFX_LOG("    Geometry [" + std::to_string(geometry_index++) + "] Primitive count: " + std::to_string(primitive_count));

        VkAccelerationStructureBuildRangeInfoKHR range{};
        range.primitiveCount = primitive_count;
        range.primitiveOffset = 0;
        range.firstVertex = 0;
        range.transformOffset = 0;

        build_ranges.push_back(range);
    }

    const uint64_t scratch_address = info.destination->m_impl->m_scratch_buffer.get()->device_address();
    if (scratch_address == 0)
    {
        OGFX_LOG_ERROR("Cannot build acceleration structure: invalid scratch address");
        return;
    }

    VkAccelerationStructureBuildGeometryInfoKHR build_info{};
    build_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    build_info.type = type;
    build_info.flags = flags;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.dstAccelerationStructure = info.destination->m_impl->m_acceleration_structure;
    build_info.geometryCount = static_cast<uint32_t>(info.destination->m_impl->m_geometries.size());
    build_info.pGeometries = info.destination->m_impl->m_geometries.data();
    build_info.scratchData.deviceAddress = scratch_address;

    std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> range_ptrs;
    range_ptrs.push_back(build_ranges.data());

    vkCmdBuildAccelerationStructuresKHR(m_impl->m_command_buffer, 1, &build_info, range_ptrs.data());

    OGFX_LOG("Acc. struct.build recorded");
}

} // namespace ogfx