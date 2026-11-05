#ifndef ORENDERER_RENDER_FEATURE_CONTEXT_HPP
#define ORENDERER_RENDER_FEATURE_CONTEXT_HPP

namespace orendergraph
{

class RenderGraph;

}

namespace orenderer
{

class RenderFeatureContext
{
  public:
    explicit RenderFeatureContext(orendergraph::RenderGraph& render_graph);

    orendergraph::RenderGraph& render_graph();
    const orendergraph::RenderGraph& render_graph() const;

  private:
    orendergraph::RenderGraph& m_render_graph;
};

} // namespace orenderer

#endif // ORENDERER_RENDER_FEATURE_CONTEXT_HPP