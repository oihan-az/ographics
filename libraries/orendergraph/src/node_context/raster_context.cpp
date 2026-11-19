#include <orendergraph/node_context/raster_context.hpp>

#include <orendergraph/render_graph_core.hpp>

namespace orendergraph
{

void RasterContext::create(RenderGraph& graph, const Desc& desc)
{
    PipelineContext::create(graph, desc);

    create_render_pass(graph, desc);
    create_framebuffers(graph, desc);
    create_shaders(graph, desc);
    create_pipeline(graph, desc);
}

void RasterContext::create_render_pass(RenderGraph& graph, const Desc& desc) 
{
    release_render_pass(graph);

    render_pass = graph.get_or_create_object<ogfx::RenderPass>(desc.render_pass_desc);
}

void RasterContext::create_framebuffers(RenderGraph& graph, const Desc& desc)
{
    release_framebuffers(graph);

    for (const ogfx::FramebufferDesc& framebuffer_desc : desc.framebuffers_desc)
    {
        ogfx::FramebufferDesc desc = framebuffer_desc;
        desc.render_pass = render_pass;
        framebuffers.push_back(graph.get_or_create_object<ogfx::Framebuffer>(desc));
    }
}

void RasterContext::create_pipeline(RenderGraph& graph, const Desc& desc)
{
    release_pipeline(graph);

    // No graphics pipeline is needed when this raster context
    // only uses the render pass to record commands
    if (vertex_shader == nullptr)
        return;

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
    pipeline_desc.depth_stencil = desc.depth_stencil;
    pipeline = graph.get_or_create_object<ogfx::GraphicsPipeline>(pipeline_desc);
}

void RasterContext::create_shaders(RenderGraph& graph, const Desc& desc)
{
    release_shaders(graph);

    vertex_shader = desc.vertex_shader_desc.bytecode.empty()
                    ? nullptr
                    : graph.get_or_create_object<ogfx::Shader>(desc.vertex_shader_desc);

    fragment_shader = desc.fragment_shader_desc.bytecode.empty()
                      ? nullptr
                      : graph.get_or_create_object<ogfx::Shader>(desc.fragment_shader_desc);
}

void RasterContext::release_render_pass(RenderGraph& graph)
{
    if (render_pass)
        graph.release_object(render_pass);
    render_pass = nullptr;
}

void RasterContext::release_framebuffers(RenderGraph& graph)
{
    if (!framebuffers.empty())
    {
        for (ogfx::Framebuffer* framebuffer : framebuffers)
            graph.release_object(framebuffer);
    }
    framebuffers.clear();
}

void RasterContext::release_pipeline(RenderGraph& graph)
{
    if (pipeline)
        graph.release_object(pipeline);
    pipeline = nullptr;
}

void RasterContext::release_shaders(RenderGraph& graph) 
{
    if (vertex_shader)
        graph.release_object(vertex_shader);
    vertex_shader = nullptr;

    if (fragment_shader)
        graph.release_object(fragment_shader);
    fragment_shader = nullptr;
}

} // namespace orendergraph
