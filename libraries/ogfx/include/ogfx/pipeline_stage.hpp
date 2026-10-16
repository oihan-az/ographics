#ifndef OGFX_PIPELINE_STAGE_HPP
#define OGFX_PIPELINE_STAGE_HPP

#include <ogfx/enum_flags.hpp>

#include <string>

namespace ogfx
{

OGFX_BITMASK_ENUM(PipelineStage, uint32_t,
    None = 0,
    TopOfPipe = 1u << 0,
    DrawIndirect = 1u << 1,
    VertexInput = 1u << 2,
    VertexShader = 1u << 3,
    FragmentShader = 1u << 4,
    ColorAttachmentOutput = 1u << 5,
    ComputeShader = 1u << 6,
    Transfer = 1u << 7,
    BottomOfPipe = 1u << 8
);

} // namespace ogfx

#endif // OGFX_PIPELINE_STAGE_HPP