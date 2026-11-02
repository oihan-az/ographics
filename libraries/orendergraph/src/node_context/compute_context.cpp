#include <orendergraph/node_context/compute_context.hpp>

#include <orendergraph/render_graph_core.hpp>

namespace orendergraph
{

void ComputeContext::create(RenderGraph& graph, const Desc& desc)
{
    // Clear
    clear();

    // Pipeline context
    PipelineContext::create(graph, desc);

    // Shader
    ogfx::Shader* compute_shader = desc.compute_shader_desc.bytecode.empty()
                                   ? nullptr
                                   : graph.get_or_create_object<ogfx::Shader>(desc.compute_shader_desc);

    // Pipeline
    ogfx::ComputePipelineDesc pipeline_desc;
    pipeline_desc.layout = pipeline_layout;
    pipeline_desc.shader = compute_shader;
    pipeline = graph.get_or_create_object<ogfx::ComputePipeline>(pipeline_desc);
}

void ComputeContext::clear()
{
    PipelineContext::clear();
    pipeline = nullptr;
}

} // namespace orendergraph
