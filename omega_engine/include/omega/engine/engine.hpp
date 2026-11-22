#ifndef OMEGA_ENGINE_HPP
#define OMEGA_ENGINE_HPP

#include <omega/core/event/event_system.hpp>
#include <omega/core/entity/entity_system.hpp>
#include <omega/core/hierarchy/hierarchy_system.hpp>
#include <omega/core/component/component_system.hpp>
#include <omega/core/performance/performance_system.hpp>

#include <omega/transform/transform_system.hpp>
#include <omega/camera/camera_system.hpp>

#include <omega/platform/platform_system.hpp>
#include <omega/graphics/graphics_system.hpp>

#include <cstdint>

namespace omega
{

struct EngineDesc
{
    std::string application_name = "Omega Engine";

    oplatform::WindowDesc window;

    uint32_t max_frames_in_flight = 2;
};

class Engine
{
  public:
    Engine(const EngineDesc& desc);

    void initialize();
    void update();
    void render();
    bool start_frame();
    void end_frame();
    void wait();

    bool should_close() const;

    EventSystem& events();
    EntitySystem& entities();
    HierarchySystem& hierarchy();
    ComponentSystem& components();
    TransformSystem& transforms();
    CameraSystem& cameras();
    PlatformSystem& platform();
    GraphicsSystem& graphics();
    PerformanceSystem& performance();

  private:
    EventSystem m_events;
    EntitySystem m_entities;
    HierarchySystem m_hierarchy;
    ComponentSystem m_components;
    TransformSystem m_transforms;
    CameraSystem m_cameras;
    PlatformSystem m_platform;
    GraphicsSystem m_graphics;
    PerformanceSystem m_performance;
};

} // namespace omega

#endif // OMEGA_ENGINE_HPP