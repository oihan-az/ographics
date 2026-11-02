#ifndef ORENDERGRAPH_NODE_COMPILE_TLAS_HPP
#define ORENDERGRAPH_NODE_COMPILE_TLAS_HPP

#include <orendergraph/node_compilation_policy.hpp>
#include <orendergraph/node_context/tlas_context.hpp>

namespace orendergraph
{

template <> 
struct NodeCompile<TLASContext>
{
    static void compile(RenderGraph& graph, TLASContext& context, const TLASContext::Desc& desc)
    {
        context.create(graph, desc);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_COMPILE_TLAS_HPP