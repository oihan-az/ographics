#ifndef ORENDERGRAPH_NODE_COMPILE_RAY_TRACING_HPP
#define ORENDERGRAPH_NODE_COMPILE_RAY_TRACING_HPP

#include <orendergraph/node_compilation_policy.hpp>
#include <orendergraph/node_context/ray_tracing_context.hpp>

namespace orendergraph
{

template <> 
struct NodeCompile<RayTracingContext>
{
    static void compile(RenderGraph& graph, RayTracingContext& context, const RayTracingContext::Desc& desc)
    {
        context.create(graph, desc);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_COMPILE_RAY_TRACING_HPP