#ifndef OGFX_GRAPHICS_PIEPLINE_HPP
#define OGFX_GRAPHICS_PIEPLINE_HPP

#include <ogfx/pipeline.hpp>

#include <ogfx/vertex_input.hpp>
#include <ogfx/input_assembly.hpp>

#include <ogfx/rasterization.hpp>
#include <ogfx/multisample.hpp>

#include <ogfx/color_blend.hpp>

#include <ogfx/viewport.hpp>

#include <memory>

namespace ogfx
{

class Device;
class RenderPass;
class Shader;
class PipelineLayout;

struct GraphicsPipelineDesc
{
    const RenderPass* render_pass = nullptr;
    uint32_t subpass = 0;

    const PipelineLayout* layout = nullptr;

    // Programmable stages
    const Shader* vertex_shader = nullptr;
    const Shader* fragment_shader = nullptr;

    // Fixed function pipeline state
    VertexInputStateDesc vertex_input;
    InputAssemblyStateDesc input_assembly;

    RasterizationStateDesc rasterization;
    MultisampleStateDesc multisampling;

    ColorBlendStateDesc color_blend;

    ViewportStateDesc viewport;
};

class GraphicsPipeline : public Pipeline
{
  public:
    GraphicsPipeline(const Device& device, const GraphicsPipelineDesc& desc);

    ~GraphicsPipeline();

    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    GraphicsPipeline(GraphicsPipeline&&) noexcept;
    GraphicsPipeline& operator=(GraphicsPipeline&&) noexcept;

    PipelineType type() const noexcept override
    {
        return PipelineType::Graphics;
    }
};

} // namespace ogfx

#endif // OGFX_GRAPHICS_PIEPLINE_HPP