#include <orenderer/render_feature_context.hpp>

namespace orenderer
{

RenderFeatureContext::RenderFeatureContext(orendergraph::RenderGraph& render_graph) : m_render_graph(render_graph) {}

orendergraph::RenderGraph& RenderFeatureContext::render_graph()
{
    return m_render_graph;
}

const orendergraph::RenderGraph& RenderFeatureContext::render_graph() const
{
    return m_render_graph;
}

} // namespace orenderer