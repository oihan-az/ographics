#ifndef ORENDERGRAPH_CONTEXT_GENERAL_HPP
#define ORENDERGRAPH_CONTEXT_GENERAL_HPP

namespace orendergraph
{

class RenderGraph;

struct GeneralContext
{
    struct Desc {};

    void create(RenderGraph& graph, const Desc& desc) {}
};

} // namespace orendergraph

#endif // ORENDERGRAPH_CONTEXT_GENERAL_HPP
