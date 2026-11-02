#ifndef ORENDERGRAPH_NODE_COMPILE_RASTER_HPP
#define ORENDERGRAPH_NODE_COMPILE_RASTER_HPP

#include <orendergraph/node_compilation_policy.hpp>
#include <orendergraph/node_context/raster_context.hpp>

namespace orendergraph
{

template <> 
struct NodeCompile<RasterContext>
{
    static void compile(RenderGraph& graph, RasterContext& context, const RasterContext::Desc& desc)
    {
        context.create(graph, desc);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_COMPILE_RASTER_HPP