#ifndef ORENDERGRAPH_NODE_COMPILE_BLAS_HPP
#define ORENDERGRAPH_NODE_COMPILE_BLAS_HPP

#include <orendergraph/node_compilation_policy.hpp>
#include <orendergraph/node_context/blas_context.hpp>

namespace orendergraph
{

template <> 
struct NodeCompile<BLASContext>
{
    static void compile(RenderGraph& graph, BLASContext& context, const BLASContext::Desc& desc)
    {
        context.create(graph, desc);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_COMPILE_BLAS_HPP