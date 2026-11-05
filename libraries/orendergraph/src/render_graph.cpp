#include <orendergraph/render_graph.hpp>

#include <optional>
#include <queue>
#include <unordered_map>

namespace orendergraph
{

RenderGraph::RenderGraph(ogfx::PhysicalDevice& pd, ogfx::Device& device, ogfx::ResourceAllocator& resource_allocator)
    : m_physical_device(&pd), m_device(&device), m_resource_allocator(&resource_allocator), m_object_cache(pd, device)
{
}

RenderGraph::~RenderGraph()
{
    reset();
}

void RenderGraph::compile()
{
    // Build resource dependencies
    build_dependencies();

    // Determine execution order
    build_execution_order();

    // TODO: Validate resource initialization
    // TODO: Cull unused nodes
}

void RenderGraph::execute(ogfx::CommandBuffer& cmd)
{
    cmd.reset();
    cmd.begin();

    for (auto& access : m_begin_frame_transitions)
    {
        access->transition(cmd, m_registry);
    }

    for (const auto& node : m_execution_order)
    {
        if (node->execution_frequency() == ExecutionFrequency::Once && node->has_executed())
        {
            continue;
        }

        for (auto& access : node->reads())
            access->transition(cmd, m_registry);

        for (auto& access : node->writes())
            access->transition(cmd, m_registry);

        node->pre_execute(*this, cmd);
        node->execute(*this, cmd);
        node->post_execute(*this, cmd);

        for (auto& access : node->writes())
        {
            access->update_state(m_registry);
        }

        if (node->execution_frequency() == ExecutionFrequency::Once)
            node->mark_executed();
    }

    for (auto& access : m_end_frame_transitions)
    {
        access->transition(cmd, m_registry);
    }

    cmd.end();
}

void RenderGraph::reset()
{
    m_nodes.clear();
    m_registry.reset();
    m_object_cache.clear();
}

void RenderGraph::add_dependency(uint32_t from, uint32_t to)
{
    if (from == to)
        return;

    auto& dependents = m_dependencies[from].dependents;

    if (std::find(dependents.begin(), dependents.end(), to) != dependents.end())
        return;

    dependents.push_back(to);
    m_dependencies[to].dependencies.push_back(from);
}

void RenderGraph::build_dependencies()
{
    m_dependencies.clear();
    m_dependencies.resize(m_nodes.size());

    struct ResourceDependency
    {
        std::optional<uint32_t> last_writer;
        std::vector<uint32_t> readers;
    };

    std::unordered_map<uint32_t, ResourceDependency> resource_dependencies;

    for (uint32_t i = 0; i < m_nodes.size(); ++i)
    {
        auto& node = m_nodes[i];

        // Reads
        for (auto& access : node->reads())
        {
            uint32_t resource = access->resource_id();
            auto& dependency = resource_dependencies[resource];

            if (dependency.last_writer.has_value())
            {
                add_dependency(*dependency.last_writer, i);
            }

            dependency.readers.push_back(i);
        }

        // Writes
        for (auto& access : node->writes())
        {
            uint32_t resource = access->resource_id();
            auto& dependency = resource_dependencies[resource];

            if (dependency.last_writer.has_value())
            {
                add_dependency(*dependency.last_writer, i);
            }

            for (uint32_t reader : dependency.readers)
            {
                add_dependency(reader, i);
            }

            dependency.readers.clear();
            dependency.last_writer = i;
        }
    }
}

void RenderGraph::build_execution_order()
{
    m_execution_order.clear();

    std::vector<uint32_t> indegree(m_nodes.size());

    std::queue<uint32_t> ready_nodes;

    // Initialize indegrees
    for (uint32_t i = 0; i < m_nodes.size(); ++i)
    {
        indegree[i] = static_cast<uint32_t>(m_dependencies[i].dependencies.size());

        if (indegree[i] == 0)
            ready_nodes.push(i);
    }

    // Topological sort
    while (!ready_nodes.empty())
    {
        uint32_t current = ready_nodes.front();
        ready_nodes.pop();

        m_execution_order.push_back(m_nodes[current].get());

        for (uint32_t dependent : m_dependencies[current].dependents)
        {
            if (--indegree[dependent] == 0)
            {
                ready_nodes.push(dependent);
            }
        }
    }

    // Cycle detection
    if (m_execution_order.size() != m_nodes.size())
    {
        m_execution_order.clear();

        throw std::runtime_error("RenderGraph: Cyclic dependency detected.");
    }
}

};