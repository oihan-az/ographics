#include "vk_acc_struct_build_preference.hpp"

namespace ogfx
{

VkBuildAccelerationStructureFlagsKHR
to_vk_acceleration_structure_build_preference(AccelerationStructureBuildPreference preference)
{
    switch (preference)
    {
    case AccelerationStructureBuildPreference::PreferFastTrace:
        return VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

    case AccelerationStructureBuildPreference::PreferFastBuild:
        return VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;

    case AccelerationStructureBuildPreference::Default:
    default:
        return 0;
    }
}

AccelerationStructureBuildPreference
from_vk_acceleration_structure_build_preference(VkBuildAccelerationStructureFlagsKHR flags)
{
    if (flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR)
        return AccelerationStructureBuildPreference::PreferFastTrace;

    if (flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR)
        return AccelerationStructureBuildPreference::PreferFastBuild;

    return AccelerationStructureBuildPreference::Default;
}

} // namespace ogfx