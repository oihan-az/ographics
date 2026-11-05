#ifndef OGFX_DEPTH_STENCIL_HPP
#define OGFX_DEPTH_STENCIL_HPP

#include <ogfx/compare_op.hpp>
#include <ogfx/stencil_op.hpp>

#include <cstdint>

namespace ogfx
{

struct StencilOpState
{
    StencilOp fail_op = StencilOp::Keep;
    StencilOp pass_op = StencilOp::Keep;
    StencilOp depth_fail_op = StencilOp::Keep;

    CompareOp compare_op = CompareOp::Always;

    uint32_t compare_mask = 0xFFFFFFFF;
    uint32_t write_mask = 0xFFFFFFFF;
    uint32_t reference = 0;
};

struct DepthStencilDesc
{
    // Depth testing
    bool depth_test_enable = true;
    bool depth_write_enable = true;
    CompareOp depth_compare_op = CompareOp::Less;
    bool depth_bounds_test_enable = false;
    float min_depth_bounds = 0.0f;
    float max_depth_bounds = 1.0f;

    // Stencil
    bool stencil_test_enable = false;
    StencilOpState front;
    StencilOpState back;
};

} // namespace ogfx

#endif // OGFX_DEPTH_STENCIL_HPP