#ifndef ORENDERGRAPH_NODE_COMPILE_COMPUTE_HPP
#define ORENDERGRAPH_NODE_COMPILE_COMPUTE_HPP

#include <orendergraph/node_compilation_policy.hpp>
#include <orendergraph/node_context/compute_context.hpp>

namespace orendergraph
{

template <> 
struct NodeCompile<ComputeContext>
{
    static void compile(RenderGraph& graph, ComputeContext& context, const ComputeContext::Desc& desc)
    {
        context.create(graph, desc);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_COMPILE_COMPUTE_HPP