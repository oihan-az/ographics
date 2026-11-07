#ifndef OMEGA_SCENE_TPP
#define OMEGA_SCENE_TPP

#include <utility>

namespace omega
{

template<typename T>
bool Scene::add(Entity entity, T component)
{
    if (!is_alive(entity))
        return false;

    if (!m_components.add<T>(entity, std::move(component)))
        return false;

    if (m_events)
        m_components.publish_added<T>(entity, *m_events);

    return true;
}

template <typename T> 
bool Scene::remove(Entity entity)
{
    if (!is_alive(entity))
        return false;

    if (!m_components.remove<T>(entity))
        return false;

    if (m_events)
        m_components.publish_removed<T>(entity, *m_events);

    return true;
}

template <typename T> 
bool Scene::mark_dirty(Entity entity)
{
    if (!is_alive(entity) || !m_components.has<T>(entity))
        return false;

    if (m_events)
        m_components.publish_changed<T>(entity, *m_events);

    return true;
}

template<typename T>
T* Scene::get(Entity entity)
{
    if (!is_alive(entity))
        return nullptr;

    return m_components.get<T>(entity);
}

template<typename T>
const T* Scene::get(Entity entity) const
{
    if (!is_alive(entity))
        return nullptr;

    return m_components.get<T>(entity);
}

template <typename T> 
bool Scene::has(Entity entity) const
{
    if (!is_alive(entity))
        return false;

    return m_components.has<T>(entity);
}

template <typename T, typename Callback> void Scene::for_each(Callback&& callback)
{
    m_components.for_each<T>(std::forward<Callback>(callback));
}

template <typename T, typename Callback> void Scene::for_each(Callback&& callback) const
{
    m_components.for_each<T>(std::forward<Callback>(callback));
}

} // namespace omega

#endif // OMEGA_SCENE_TPP