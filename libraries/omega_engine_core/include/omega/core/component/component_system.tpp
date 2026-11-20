#ifndef OMEGA_COMPONENT_SYSTEM_TPP
#define OMEGA_COMPONENT_SYSTEM_TPP

#include <omega/core/event/event_system.hpp>

#include <utility>

namespace omega
{

template <typename T> 
ComponentStorage<T>& ComponentSystem::get_storage()
{
    std::type_index type = typeid(T);

    auto it = m_storages.find(type);
    if (it != m_storages.end())
        return *static_cast<ComponentStorage<T>*>(it->second.get());

    auto storage = std::make_unique<ComponentStorage<T>>(m_events);
    ComponentStorage<T>& reference = *storage;

    m_storages.emplace(type, std::move(storage));

    // Automatically register the component and its dependencies
    register_component_type<T>();

    return reference;
}

template <typename T> 
bool ComponentSystem::remove_storage()
{
    if (!has_storage<T>())
        return false;

    const auto entities = entities_with<T>();
    for (Entity entity : entities)
        remove<T>(entity);

    return m_storages.erase(std::type_index(typeid(T))) > 0;
}

template <typename T> 
bool ComponentSystem::has_storage() const
{
    return m_storages.contains(std::type_index(typeid(T)));
}

template <typename T> 
bool ComponentSystem::add(Entity entity, T component)
{
    if (!m_entities.is_valid(entity))
        return false;

    // Creates the storage and registers dependencies if needed
    auto& storage = get_storage<T>();
    if (storage.contains(entity))
        return false;

    // Track newly added dependencies for rollback
    const auto order = m_dependency_graph.dependency_order(std::type_index(typeid(T)));
    std::vector<std::type_index> added;
    for (std::size_t i = 0; i + 1 < order.size(); ++i)
    {
        const auto type = order[i];
        const auto* node = m_dependency_graph.find(type);
        if (node->operations.has(entity))
            continue;

        if (!node->operations.ensure(entity))
        {
            for (auto it = added.rbegin(); it != added.rend(); ++it)
            {
                m_dependency_graph.find(*it)->operations.remove(entity);
            }

            return false;
        }

        added.push_back(type);
    }

    if (storage.add(entity, std::move(component)))
        return true;

    for (auto it = added.rbegin(); it != added.rend(); ++it)
    {
        m_dependency_graph.find(*it)->operations.remove(entity);
    }

    return false;
}

template <typename T> 
bool ComponentSystem::remove(Entity entity)
{
    if (!has<T>(entity))
        return false;

    const auto order = m_dependency_graph.removal_order(std::type_index(typeid(T)));

    for (const auto& type : order)
    {
        const auto* node = m_dependency_graph.find(type);
        if (node->operations.has(entity))
            node->operations.remove(entity);
    }

    return get_storage<T>().remove(entity);
}

template <typename T> 
bool ComponentSystem::mark_dirty(Entity entity, ComponentModifiedOrigin origin)
{
    if (!m_entities.is_valid(entity))
        return false;

    if (!has<T>(entity))
        return false;

    // Reverse removal order: changed component first
    const auto order = m_dependency_graph.removal_order(std::type_index(typeid(T)));
    for (auto it = order.rbegin(); it != order.rend(); ++it)
    {
        const auto* node = m_dependency_graph.find(*it);
        if (node->operations.has(entity))
            node->operations.mark_dirty(entity, origin);
    }

    return get_storage<T>().mark_dirty(entity);
}

template<typename T>
bool ComponentSystem::has(Entity entity) const
{
    auto it = m_storages.find(std::type_index(typeid(T)));
    if (it == m_storages.end())
        return false;

    const auto* storage = static_cast<const ComponentStorage<T>*>(it->second.get());
    return storage->contains(entity);
}

template<typename T>
T* ComponentSystem::get(Entity entity)
{
    if (!has_storage<T>())
        return nullptr;

    return get_storage<T>().get(entity);
}

template<typename T>
const T* ComponentSystem::get(Entity entity) const
{
    auto it = m_storages.find(std::type_index(typeid(T)));
    if (it == m_storages.end())
        return nullptr;

    const auto* storage = static_cast<const ComponentStorage<T>*>(it->second.get());
    return storage->get(entity);
}

template <typename T>
std::vector<Entity> ComponentSystem::entities_with() const
{
    auto it = m_storages.find(std::type_index(typeid(T)));
    if (it == m_storages.end())
        return {};

    const auto* storage = static_cast<const ComponentStorage<T>*>(it->second.get());

    return storage->entities();
}

template <typename T> 
void ComponentSystem::register_component_type()
{
    const ComponentDependencyGraph::Type type = typeid(T);
    if (m_dependency_graph.is_registered(type))
        return;

    ComponentDependencyGraph::Operations operations;
    operations.has = [this](Entity entity) { return has<T>(entity); };
    operations.ensure = [this](Entity entity)
    {
        if (has<T>(entity))
            return true;

        return get_storage<T>().add(entity, T{});
    };
    operations.remove = [this](Entity entity)
    {
        if (!has<T>(entity))
            return false;

        return get_storage<T>().remove(entity);
    };
    operations.mark_dirty = [this](Entity entity, ComponentModifiedOrigin origin)
    {
        if (!has<T>(entity))
            return false;

        return get_storage<T>().mark_dirty(entity, origin);
    };

    m_dependency_graph.register_type(type, std::move(operations));

    register_dependencies<T>(typename ComponentTraits<T>::Dependencies{});
}

template <typename T, typename... Deps> 
void ComponentSystem::register_dependencies(TypeList<Deps...>)
{
    // Ensure all dependency storages exist
    (get_storage<Deps>(), ...);

    // Register the relationships in the graph
    (m_dependency_graph.add_dependency(std::type_index(typeid(T)), std::type_index(typeid(Deps))), ...);
}

} // namespace omega

#endif // OMEGA_COMPONENT_SYSTEM_TPP