#include "vk_compare_op.hpp"

namespace ogfx
{

VkCompareOp to_vk_compare_op(CompareOp op)
{
    switch (op)
    {
    case CompareOp::Undefined:
        return VK_COMPARE_OP_MAX_ENUM;
    case CompareOp::Never:
        return VK_COMPARE_OP_NEVER;
    case CompareOp::Less:
        return VK_COMPARE_OP_LESS;
    case CompareOp::Equal:
        return VK_COMPARE_OP_EQUAL;
    case CompareOp::LessOrEqual:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case CompareOp::Greater:
        return VK_COMPARE_OP_GREATER;
    case CompareOp::NotEqual:
        return VK_COMPARE_OP_NOT_EQUAL;
    case CompareOp::GreaterOrEqual:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case CompareOp::Always:
        return VK_COMPARE_OP_ALWAYS;
    default:
        return VK_COMPARE_OP_MAX_ENUM;
    }
}

CompareOp from_vk_compare_op(VkCompareOp op)
{
    switch (op)
    {
    case VK_COMPARE_OP_NEVER:
        return CompareOp::Never;
    case VK_COMPARE_OP_LESS:
        return CompareOp::Less;
    case VK_COMPARE_OP_EQUAL:
        return CompareOp::Equal;
    case VK_COMPARE_OP_LESS_OR_EQUAL:
        return CompareOp::LessOrEqual;
    case VK_COMPARE_OP_GREATER:
        return CompareOp::Greater;
    case VK_COMPARE_OP_NOT_EQUAL:
        return CompareOp::NotEqual;
    case VK_COMPARE_OP_GREATER_OR_EQUAL:
        return CompareOp::GreaterOrEqual;
    case VK_COMPARE_OP_ALWAYS:
        return CompareOp::Always;
    default:
        return CompareOp::Undefined;
    }
}

} // namespace ogfx