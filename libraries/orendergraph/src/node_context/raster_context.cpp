#include <orendergraph/node_context/raster_context.hpp>

#include <orendergraph/render_graph_core.hpp>

namespace orendergraph
{

void RasterContext::create(RenderGraph& graph, const Desc& desc)
{
    // Clear
    clear();

    // Pipeline context
    PipelineContext::create(graph, desc);

    // Render pass
    render_pass = graph.get_or_create_object<ogfx::RenderPass>(desc.render_pass_desc);

    // Frame buffers
    for (const ogfx::FramebufferDesc& framebuffer_desc : desc.framebuffers_desc)
    {
        ogfx::FramebufferDesc desc = framebuffer_desc;
        desc.render_pass = render_pass;
        framebuffers.push_back(graph.get_or_create_object<ogfx::Framebuffer>(desc));
    }

    // Shaders
    ogfx::Shader* vertex_shader = desc.vertex_shader_desc.bytecode.empty()
                                  ? nullptr
                                  : graph.get_or_create_object<ogfx::Shader>(desc.vertex_shader_desc);

    ogfx::Shader* fragment_shader = desc.fragment_shader_desc.bytecode.empty()
                                    ? nullptr
                                    : graph.get_or_create_object<ogfx::Shader>(desc.fragment_shader_desc);

    // Pipeline
    ogfx::GraphicsPipelineDesc pipeline_desc;
    pipeline_desc.render_pass = render_pass;
    pipeline_desc.layout = pipeline_layout;
    pipeline_desc.vertex_shader = vertex_shader;
    pipeline_desc.fragment_shader = fragment_shader;
    pipeline_desc.vertex_input = desc.vertex_input;
    pipeline_desc.input_assembly = desc.input_assembly;
    pipeline_desc.viewport = desc.viewport;
    pipeline_desc.rasterization = desc.rasterization;
    pipeline_desc.multisampling = desc.multisampling;
    pipeline_desc.color_blend = desc.color_blend;
    pipeline = graph.get_or_create_object<ogfx::GraphicsPipeline>(pipeline_desc);
}

void RasterContext::clear()
{
    PipelineContext::clear();
    pipeline = nullptr;
    render_pass = nullptr;
    framebuffers.clear();
}

} // namespace orendergraph
