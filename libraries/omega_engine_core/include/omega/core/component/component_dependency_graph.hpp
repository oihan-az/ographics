#ifndef OMEGA_COMPONENT_DEPENDENCY_GRAPH_HPP
#define OMEGA_COMPONENT_DEPENDENCY_GRAPH_HPP

#include <omega/core/entity/entity.hpp>
#include <omega/core/component/component_event.hpp>

#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace omega
{

class ComponentDependencyGraph
{
  public:
    using Type = std::type_index;

    struct Operations
    {
        std::function<bool(Entity)> has;
        std::function<bool(Entity)> ensure;
        std::function<bool(Entity)> remove;
        std::function<bool(Entity, ComponentModifiedOrigin)> mark_dirty;
    };

    struct Node
    {
        std::vector<Type> dependencies;
        std::vector<Type> dependants;
        Operations operations;
        bool registered = false;
    };

    void register_type(Type type, Operations operations);
    void add_dependency(Type type, Type dependency);

    bool is_registered(Type type) const;

    const Node* find(Type type) const;

    std::vector<Type> dependency_order(Type type) const;
    std::vector<Type> removal_order(Type type) const;

  private:
    bool reaches(Type from, Type target) const;
    std::vector<Type> traverse(Type type, bool reverse) const;

    std::unordered_map<Type, Node> m_nodes;
};

} // namespace omega

#endif // OMEGA_COMPONENT_DEPENDENCY_GRAPH_HPP