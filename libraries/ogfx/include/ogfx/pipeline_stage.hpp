#ifndef OGFX_PIPELINE_STAGE_HPP
#define OGFX_PIPELINE_STAGE_HPP

#include <string>

namespace ogfx
{

enum class PipelineStage : uint32_t
{
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
};

constexpr PipelineStage operator|(PipelineStage lhs, PipelineStage rhs)
{
    return static_cast<PipelineStage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr bool has_flag(PipelineStage value, PipelineStage flag)
{
    return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

std::string to_string(PipelineStage stage);

} // namespace ogfx

#endif // OGFX_PIPELINE_STAGE_HPP