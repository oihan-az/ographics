#include "vk_stencil_op.hpp"

namespace ogfx
{

VkStencilOp to_vk_stencil_op(StencilOp op)
{
    switch (op)
    {
    case StencilOp::Undefined:
        return VK_STENCIL_OP_MAX_ENUM;
    case StencilOp::Keep:
        return VK_STENCIL_OP_KEEP;
    case StencilOp::Zero:
        return VK_STENCIL_OP_ZERO;
    case StencilOp::Replace:
        return VK_STENCIL_OP_REPLACE;
    case StencilOp::IncrementAndClamp:
        return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case StencilOp::DecrementAndClamp:
        return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case StencilOp::Invert:
        return VK_STENCIL_OP_INVERT;
    case StencilOp::IncrementAndWrap:
        return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case StencilOp::DecrementAndWrap:
        return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    default:
        return VK_STENCIL_OP_MAX_ENUM;
    }
}

StencilOp from_vk_stencil_op(VkStencilOp op)
{
    switch (op)
    {
    case VK_STENCIL_OP_KEEP:
        return StencilOp::Keep;
    case VK_STENCIL_OP_ZERO:
        return StencilOp::Zero;
    case VK_STENCIL_OP_REPLACE:
        return StencilOp::Replace;
    case VK_STENCIL_OP_INCREMENT_AND_CLAMP:
        return StencilOp::IncrementAndClamp;
    case VK_STENCIL_OP_DECREMENT_AND_CLAMP:
        return StencilOp::DecrementAndClamp;
    case VK_STENCIL_OP_INVERT:
        return StencilOp::Invert;
    case VK_STENCIL_OP_INCREMENT_AND_WRAP:
        return StencilOp::IncrementAndWrap;
    case VK_STENCIL_OP_DECREMENT_AND_WRAP:
        return StencilOp::DecrementAndWrap;
    default:
        return StencilOp::Undefined;
    }
}


} // namespace ogfx