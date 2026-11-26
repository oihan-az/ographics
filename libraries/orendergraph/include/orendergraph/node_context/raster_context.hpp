#ifndef ORENDERGRAPH_CONTEXT_RASTER_HPP
#define ORENDERGRAPH_CONTEXT_RASTER_HPP

#include <orendergraph/node_context/pipeline_context.hpp>

#include <ogfx/render_pass.hpp>
#include <ogfx/framebuffer.hpp>
#include <ogfx/shader.hpp>
#include <ogfx/vertex_input.hpp>
#include <ogfx/input_assembly.hpp>
#include <ogfx/rasterization.hpp>
#include <ogfx/multisample.hpp>
#include <ogfx/color_blend.hpp>
#include <ogfx/viewport.hpp>
#include <ogfx/depth_stencil.hpp>
#include <ogfx/commands/render_pass_commands.hpp>

#include <vector>

namespace ogfx
{

class Shader;
class GraphicsPipeline;
class RenderPass;
class Framebuffer;

} // namespace ogfx

namespace orendergraph
{

class RenderGraph;

struct RasterContext : public PipelineContext
{
    struct Desc : public PipelineContext::Desc
    {
        ogfx::RenderPassDesc render_pass_desc;
        std::vector<ogfx::FramebufferDesc> framebuffers_desc;

        ogfx::ShaderDesc vertex_shader_desc;
        ogfx::ShaderDesc fragment_shader_desc;

        ogfx::VertexInputStateDesc vertex_input;
        ogfx::InputAssemblyStateDesc input_assembly;
        ogfx::RasterizationStateDesc rasterization;
        ogfx::MultisampleStateDesc multisampling;
        ogfx::ColorBlendStateDesc color_blend;
        ogfx::ViewportStateDesc viewport;
        ogfx::DepthStencilDesc depth_stencil;

        uint32_t framebuffer_index = 0;

        uint32_t offset_x = 0;
        uint32_t offset_y = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        std::vector<ogfx::ClearValue> clear_values;
    };

    void create(RenderGraph& graph, const Desc& desc);

    void create_render_pass(RenderGraph& graph, const Desc& desc);
    void create_framebuffers(RenderGraph& graph, const Desc& desc);
    void create_pipeline(RenderGraph& graph, const Desc& desc);
    void create_shaders(RenderGraph& graph, const Desc& desc);

    void release_render_pass(RenderGraph& graph);
    void release_framebuffers(RenderGraph& graph);
    void release_pipeline(RenderGraph& graph);
    void release_shaders(RenderGraph& graph);

    uint32_t add_framebuffer(RenderGraph& graph, Desc& desc, const ogfx::FramebufferDesc& framebuffer_desc);
    void remove_framebuffer(RenderGraph& graph, Desc& desc, uint32_t index);

    ogfx::Shader* vertex_shader = nullptr;
    ogfx::Shader* fragment_shader = nullptr;
    ogfx::GraphicsPipeline* pipeline = nullptr;
    ogfx::RenderPass* render_pass = nullptr;
    std::vector<ogfx::Framebuffer*> framebuffers;

private:
    ogfx::Framebuffer* acquire_framebuffer(RenderGraph& graph, const ogfx::FramebufferDesc& framebuffer_desc);
    void release_framebuffer_slot(RenderGraph& graph, uint32_t index);
};

} // namespace orendergraph

#endif // ORENDERGRAPH_CONTEXT_RASTER_HPP
