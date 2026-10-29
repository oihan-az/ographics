#ifndef OGFX_VK_ACCELERATION_STRUCTURE_HPP
#define OGFX_VK_ACCELERATION_STRUCTURE_HPP

#include <ogfx/acc_struct.hpp>

#include <vulkan/vulkan.h>

#include <vector>

namespace ogfx
{

class Buffer;

struct AccelerationStructure::Impl
{
    VkDevice m_device = VK_NULL_HANDLE;
    VkAccelerationStructureKHR m_acceleration_structure = VK_NULL_HANDLE;

    std::vector<VkAccelerationStructureGeometryKHR> m_geometries;
    std::vector<uint32_t> m_primitive_counts;
    VkAccelerationStructureBuildSizesInfoKHR m_sizes{};
    VkBuildAccelerationStructureFlagsKHR m_build_flags{};

    std::unique_ptr<Buffer> m_storage_buffer;
    std::unique_ptr<Buffer> m_scratch_buffer;
};

} // namespace ogfx

#endif // OGFX_VK_ACCELERATION_STRUCTURE_HPP