#ifndef ORENDERGRAPH_NODE_EXECUTION_POLICY_HPP
#define ORENDERGRAPH_NODE_EXECUTION_POLICY_HPP

namespace orendergraph
{

class RenderGraph;

template <typename TContext>
class RenderGraphNode;

template <typename TContext> 
struct NodePreExecute
{
    static void execute(RenderGraph&, ogfx::CommandBuffer&, RenderGraphNode<TContext>&) {}
};

template <typename TContext> 
struct NodeExecute
{
    static void execute(RenderGraph&, ogfx::CommandBuffer&, RenderGraphNode<TContext>&) {}
};

template <typename TContext> 
struct NodePostExecute
{
    static void execute(RenderGraph&, ogfx::CommandBuffer&, RenderGraphNode<TContext>&) {}
};

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_CONTEXTS_HPP
