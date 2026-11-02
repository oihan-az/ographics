#ifndef ORENDERGRAPH_NODE_HPP
#define ORENDERGRAPH_NODE_HPP

#include <orendergraph/resource_access.hpp>
#include <orendergraph/node_compilation_policy.hpp>
#include <orendergraph/node_execution_policy.hpp>

#include <functional>
#include <string>
#include <vector>

namespace orendergraph
{

class RenderGraph;

enum class ExecutionFrequency
{
    EveryExecution,
    Once
};

class IRenderGraphNode
{
  public:
    virtual ~IRenderGraphNode() = default;

    const std::string& name() const { return m_name; }

    ExecutionFrequency execution_frequency() const { return m_exe_frec; }
    void set_execution_frequency(ExecutionFrequency exe_frec) { m_exe_frec = exe_frec; }

    bool has_executed() const { return m_executed; }
    void mark_executed() { m_executed = true; }

    std::vector<std::unique_ptr<IResourceAccess>>& reads() { return m_reads; }
    std::vector<std::unique_ptr<IResourceAccess>>& writes() { return m_writes; }

    virtual void pre_execute(RenderGraph& rg, ogfx::CommandBuffer& cmd) = 0;
    virtual void execute(RenderGraph& rg, ogfx::CommandBuffer& cmd) = 0;
    virtual void post_execute(RenderGraph& rg, ogfx::CommandBuffer& cmd) = 0;

protected:
    std::string m_name;
    ExecutionFrequency m_exe_frec;
    bool m_executed{false};

    std::vector<std::unique_ptr<IResourceAccess>> m_reads;
    std::vector<std::unique_ptr<IResourceAccess>> m_writes;
};

template <typename TContext>
class RenderGraphNode : public IRenderGraphNode
{
public:
    using TDesc = typename TContext::Desc;
    using SetupCallback = std::function<void(RenderGraph&, TDesc&)>;
    using ExecuteCallback = std::function<void(RenderGraph&, ogfx::CommandBuffer&, RenderGraphNode<TContext>&)>;

    RenderGraphNode(const std::string& name, ExecutionFrequency exe_frec = ExecutionFrequency::EveryExecution);

    TContext& context();
    const TContext& context() const;

    TDesc& descriptor();
    const TDesc& descriptor() const;

    void compile(RenderGraph& graph);
    void set_pre_execute(ExecuteCallback callback);
    void set_execute(ExecuteCallback callback);
    void set_post_execute(ExecuteCallback callback);

    void pre_execute(RenderGraph& graph, ogfx::CommandBuffer& cmd) override;
    void execute(RenderGraph& graph, ogfx::CommandBuffer& cmd) override;
    void post_execute(RenderGraph& graph, ogfx::CommandBuffer& cmd) override;

private:
    TContext m_ctx{};
    TDesc m_desc{};

    ExecuteCallback m_custom_pre_execute;
    ExecuteCallback m_custom_execute;
    ExecuteCallback m_custom_post_execute;
};

} // namespace orendergraph

#include <orendergraph/node.tpp>

#endif // ORENDERGRAPH_NODE_HPP