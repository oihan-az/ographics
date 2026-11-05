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

    return m_components.add<T>(entity, std::move(component));
}

template <typename T> bool Scene::remove(Entity entity)
{
    if (!is_alive(entity))
        return false;

    return m_components.remove<T>(entity);
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

template <typename T> bool Scene::has(Entity entity) const
{
    if (!is_alive(entity))
        return false;

    return m_components.has<T>(entity);
}

} // namespace omega

#endif // OMEGA_SCENE_TPP