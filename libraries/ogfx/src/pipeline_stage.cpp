#include <ogfx/pipeline_stage.hpp>

namespace ogfx
{

std::string to_string(PipelineStage stage)
{
    if (stage == PipelineStage::None)
    {
        return "None";
    }

    std::string result;

    auto append = [&](PipelineStage flag, const char* name)
    {
        if (!has_flag(stage, flag))
        {
            return;
        }

        if (!result.empty())
        {
            result += " | ";
        }

        result += name;
    };

    append(PipelineStage::TopOfPipe, "TopOfPipe");
    append(PipelineStage::DrawIndirect, "DrawIndirect");
    append(PipelineStage::VertexInput, "VertexInput");
    append(PipelineStage::VertexShader, "VertexShader");
    append(PipelineStage::FragmentShader, "FragmentShader");
    append(PipelineStage::ColorAttachmentOutput, "ColorAttachmentOutput");
    append(PipelineStage::ComputeShader, "ComputeShader");
    append(PipelineStage::Transfer, "Transfer");
    append(PipelineStage::BottomOfPipe, "BottomOfPipe");

    return result;
}

} // namespace ogfx