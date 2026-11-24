#include <orendergraph/node_context/compute_context.hpp>

#include <orendergraph/render_graph_core.hpp>

namespace orendergraph
{

void ComputeContext::create(RenderGraph& graph, const Desc& desc)
{
    PipelineContext::create(graph, desc);
    create_shader(graph, desc);
    create_pipeline(graph, desc);
}

void ComputeContext::create_pipeline(RenderGraph& graph, const Desc& desc)
{
    release_pipeline(graph);

    ogfx::ComputePipelineDesc pipeline_desc;
    pipeline_desc.layout = pipeline_layout;
    pipeline_desc.shader = compute_shader;
    pipeline = graph.get_or_create_object<ogfx::ComputePipeline>(pipeline_desc);
}

void ComputeContext::create_shader(RenderGraph& graph, const Desc& desc)
{
    release_shader(graph);

    compute_shader = desc.compute_shader_desc.bytecode.empty()
                     ? nullptr
                     : graph.get_or_create_object<ogfx::Shader>(desc.compute_shader_desc);
}

void ComputeContext::release_pipeline(RenderGraph& graph)
{
    if (pipeline)
        graph.release_object(pipeline);
    pipeline = nullptr;
}

void ComputeContext::release_shader(RenderGraph& graph)
{
    if (compute_shader)
        graph.release_object(compute_shader);
    compute_shader = nullptr;
}

} // namespace orendergraph
