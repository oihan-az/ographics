#include "vk_acc_struct_type.hpp"

namespace ogfx
{

VkAccelerationStructureTypeKHR to_vk_acceleration_structure_type(AccelerationStructureType type)
{
    switch (type)
    {
    case AccelerationStructureType::BottomLevel:
        return VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    case AccelerationStructureType::TopLevel:
        return VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

    case AccelerationStructureType::Undefined:
    default:
        return VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR;
    }
}

AccelerationStructureType from_vk_acceleration_structure_type(VkAccelerationStructureTypeKHR type)
{
    switch (type)
    {
    case VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR:
        return AccelerationStructureType::BottomLevel;

    case VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR:
        return AccelerationStructureType::TopLevel;

    default:
        return AccelerationStructureType::Undefined;
    }
}

} // namespace ogfx