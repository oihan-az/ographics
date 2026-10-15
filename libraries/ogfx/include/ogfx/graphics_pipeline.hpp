#ifndef OGFX_GRAPHICS_PIEPLINE_HPP
#define OGFX_GRAPHICS_PIEPLINE_HPP

#include <ogfx/vertex_input.hpp>
#include <ogfx/input_assembly.hpp>

#include <ogfx/rasterization.hpp>
#include <ogfx/multisample.hpp>

#include <ogfx/color_blend.hpp>

#include <ogfx/viewport.hpp>

#include <memory>
#include <vector>

namespace ogfx
{

class Device;
class RenderPass;
class Shader;

struct GraphicsPipelineDesc
{
    const RenderPass* render_pass = nullptr;
    uint32_t subpass = 0;

    // Programmable stages
    std::vector<const Shader*> shaders;

    // Fixed function pipeline state
    VertexInputStateDesc vertex_input;
    InputAssemblyStateDesc input_assembly;

    RasterizationStateDesc rasterization;
    MultisampleStateDesc multisampling;

    ColorBlendStateDesc color_blend;

    ViewportStateDesc viewport;
};

class GraphicsPipeline
{
  public:
    GraphicsPipeline(const Device& device, const GraphicsPipelineDesc& desc);

    ~GraphicsPipeline();

    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    GraphicsPipeline(GraphicsPipeline&&) noexcept;
    GraphicsPipeline& operator=(GraphicsPipeline&&) noexcept;

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    friend class CommandBuffer;
};

} // namespace ogfx

#endif // OGFX_GRAPHICS_PIEPLINE_HPP