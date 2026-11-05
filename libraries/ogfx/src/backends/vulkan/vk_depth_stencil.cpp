#include "vk_depth_stencil.hpp"
#include "vk_compare_op.hpp"
#include "vk_stencil_op.hpp"

namespace ogfx
{

VkStencilOpState to_vk_stencil_op_state(const StencilOpState& state)
{
    VkStencilOpState vk_state{};
    vk_state.failOp = to_vk_stencil_op(state.fail_op);
    vk_state.passOp = to_vk_stencil_op(state.pass_op);
    vk_state.depthFailOp = to_vk_stencil_op(state.depth_fail_op);
    vk_state.compareOp = to_vk_compare_op(state.compare_op);
    vk_state.compareMask = state.compare_mask;
    vk_state.writeMask = state.write_mask;
    vk_state.reference = state.reference;

    return vk_state;
}

StencilOpState from_vk_stencil_op_state(const VkStencilOpState& state)
{
    StencilOpState desc{};
    desc.fail_op = from_vk_stencil_op(state.failOp);
    desc.pass_op = from_vk_stencil_op(state.passOp);
    desc.depth_fail_op = from_vk_stencil_op(state.depthFailOp);
    desc.compare_op = from_vk_compare_op(state.compareOp);
    desc.compare_mask = state.compareMask;
    desc.write_mask = state.writeMask;
    desc.reference = state.reference;

    return desc;
}


} // namespace ogfx