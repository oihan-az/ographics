#ifndef OGFX_COMPUTE_PIPELINE_HPP
#define OGFX_COMPUTE_PIPELINE_HPP

#include <ogfx/pipeline.hpp>

#include <memory>

namespace ogfx
{

class Device;
class Shader;
class PipelineLayout;

struct ComputePipelineDesc
{
    const PipelineLayout* layout = nullptr;
    const Shader* shader = nullptr;
};

class ComputePipeline : public Pipeline
{
  public:
    ComputePipeline(const Device& device, const ComputePipelineDesc& desc);

    ~ComputePipeline();

    ComputePipeline(const ComputePipeline&) = delete;
    ComputePipeline& operator=(const ComputePipeline&) = delete;

    ComputePipeline(ComputePipeline&&) noexcept;
    ComputePipeline& operator=(ComputePipeline&&) noexcept;

    PipelineType type() const noexcept override
    {
        return PipelineType::Compute;
    }
};

} // namespace ogfx

#endif // OGFX_COMPUTE_PIPELINE_HPP