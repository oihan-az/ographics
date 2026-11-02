#ifndef ORENDERGRAPH_NODE_TPP
#define ORENDERGRAPH_NODE_TPP

namespace orendergraph
{

template <typename TContext>
RenderGraphNode<TContext>::RenderGraphNode(const std::string& name, ExecutionFrequency frequency)
{
    m_name = name;
    m_exe_frec = frequency;
}

template <typename TContext> 
TContext& RenderGraphNode<TContext>::context()
{
    return m_ctx;
}

template <typename TContext> 
const TContext& RenderGraphNode<TContext>::context() const
{
    return m_ctx;
}

template <typename TContext> 
typename RenderGraphNode<TContext>::TDesc& RenderGraphNode<TContext>::descriptor()
{
    return m_desc;
}

template <typename TContext>
const typename RenderGraphNode<TContext>::TDesc& RenderGraphNode<TContext>::descriptor() const
{
    return m_desc;
}

template <typename TContext> 
void RenderGraphNode<TContext>::compile(RenderGraph& graph)
{
    NodeCompile<TContext>::compile(graph, m_ctx, m_desc);
}

template <typename TContext> 
void RenderGraphNode<TContext>::set_pre_execute(ExecuteCallback callback)
{
    m_custom_pre_execute = std::move(callback);
}

template <typename TContext> 
void RenderGraphNode<TContext>::set_execute(ExecuteCallback callback)
{
    m_custom_execute = std::move(callback);
}

template <typename TContext> 
void RenderGraphNode<TContext>::set_post_execute(ExecuteCallback callback)
{
    m_custom_post_execute = std::move(callback);
}

template <typename TContext> 
void RenderGraphNode<TContext>::pre_execute(RenderGraph& graph, ogfx::CommandBuffer& cmd)
{
    if (m_custom_pre_execute)
        m_custom_pre_execute(graph, cmd, *this);
    else
        NodePreExecute<TContext>::execute(graph, cmd, *this);
}

template <typename TContext> 
void RenderGraphNode<TContext>::execute(RenderGraph& graph, ogfx::CommandBuffer& cmd)
{
    if (m_custom_execute)
        m_custom_execute(graph, cmd, *this);
    else
        NodeExecute<TContext>::execute(graph, cmd, *this);
}

template <typename TContext> 
void RenderGraphNode<TContext>::post_execute(RenderGraph& graph, ogfx::CommandBuffer& cmd)
{
    if (m_custom_post_execute)
        m_custom_post_execute(graph, cmd, *this);
    else
        NodePostExecute<TContext>::execute(graph, cmd, *this);
}

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_TPP