#include <omega/core/component/component_dependency_graph.hpp>

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace omega
{

void ComponentDependencyGraph::register_type(Type type, Operations operations)
{
    Node& node = m_nodes[type];
    if (node.registered)
        return;

    node.operations = std::move(operations);
    node.registered = true;
}

void ComponentDependencyGraph::add_dependency(Type type, Type dependency)
{
    Node& node = m_nodes[type];
    if (std::find(node.dependencies.begin(), node.dependencies.end(), dependency) != node.dependencies.end())
        return;

    if (type == dependency || reaches(dependency, type))
        throw std::logic_error("Circular component dependency");

    node.dependencies.push_back(dependency);
    m_nodes[dependency].dependants.push_back(type);
}

bool ComponentDependencyGraph::is_registered(Type type) const
{
    const Node* node = find(type);
    return node && node->registered;
}

const ComponentDependencyGraph::Node* ComponentDependencyGraph::find(Type type) const
{
    auto it = m_nodes.find(type);
    if (it == m_nodes.end())
        return nullptr;

    return &it->second;
}

bool ComponentDependencyGraph::reaches(Type from, Type target) const
{
    std::unordered_set<Type> visited;
    std::function<bool(Type)> visit = [&](Type current)
    {
        if (current == target)
            return true;

        if (!visited.insert(current).second)
            return false;

        const Node* node = find(current);

        if (!node)
            return false;

        for (Type dependency : node->dependencies)
        {
            if (visit(dependency))
                return true;
        }

        return false;
    };

    return visit(from);
}

std::vector<ComponentDependencyGraph::Type> ComponentDependencyGraph::traverse(Type type, bool reverse) const
{
    std::vector<Type> result;
    std::unordered_set<Type> visited;

    std::function<void(Type)> visit = [&](Type current)
    {
        if (!visited.insert(current).second)
            return;

        const Node* node = find(current);
        if (!node || !node->registered)
            throw std::logic_error("Component type is not registered");

        const auto& edges = reverse ? node->dependants : node->dependencies;
        for (Type next : edges)
            visit(next);

        result.push_back(current);
    };

    visit(type);

    return result;
}

std::vector<ComponentDependencyGraph::Type> ComponentDependencyGraph::dependency_order(Type type) const
{
    return traverse(type, false);
}

std::vector<ComponentDependencyGraph::Type> ComponentDependencyGraph::removal_order(Type type) const
{
    return traverse(type, true);
}

} // namespace omega