#ifndef ORENDERGRAPH_NODE_COMPILATION_POLICY_HPP
#define ORENDERGRAPH_NODE_COMPILATION_POLICY_HPP

namespace orendergraph
{

class RenderGraph;

template <typename TContext> 
struct NodeCompile
{
    static void compile(RenderGraph&, TContext&, const typename TContext::Desc&) {}
};

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_COMPILATION_POLICY_HPP
