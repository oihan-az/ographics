#ifndef OMEGA_TRANSFORM_SYSTEM_HPP
#define OMEGA_TRANSFORM_SYSTEM_HPP

#include <omega/core/entity/entity_data_cache.hpp>
#include <omega/core/event/event_system.hpp>
#include <omega/core/component/component_event.hpp>
#include <omega/core/scene/scene.hpp>
#include <omega/core/scene/hierarchy_event.hpp>
#include <omega/core/math/matrix4.hpp>

#include <omega/transform/transform.hpp>

namespace omega
{

class TransformSystem
{
  public:
    explicit TransformSystem(Scene& scene, EventSystem& events);
    ~TransformSystem();

    Matrix4 get_world_matrix(Entity entity);

  private:

    void invalidate(Entity entity);
    void on_component_event(const ComponentEvent<Transform>& event);
    void on_hierarchy_event(const HierarchyEvent& event);

  private:
    EntityDataCache<Matrix4> m_cache;

    Scene& m_scene;
    EventSystem& m_events;

    uint64_t m_component_subscription = 0;
    uint64_t m_hierarchy_subscription = 0;
};

} // namespace omega

#endif // OMEGA_TRANSFORM_SYSTEM_HPP