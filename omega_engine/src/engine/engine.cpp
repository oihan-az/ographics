#include <omega/engine/engine.hpp>

namespace omega
{

Engine::Engine(const EngineDesc& desc)
    : m_entities(m_events), 
      m_hierarchy(m_events, m_entities),
      m_components(m_events, m_entities), 
      m_transforms(m_events, m_components, m_hierarchy, m_entities),
      m_cameras(m_events, m_components, m_hierarchy, m_entities, m_transforms),
      m_platform(desc.window),
      m_graphics(desc.application_name, desc.max_frames_in_flight, m_platform, m_events, m_performance)
{}

void Engine::initialize()
{
    m_graphics.initialize();
}

void Engine::update()
{
    ScopedCpuTimer timer(m_performance, "Engine Update");

    {
        ScopedCpuTimer timer(m_performance, "Events");
        m_events.swap_all_buffers();
    }

    {
        ScopedCpuTimer timer(m_performance, "Entities");
        m_entities.update();
    }

    {
        ScopedCpuTimer timer(m_performance, "Transform hierarchy");
        m_transforms.prepare_hierarchy_changes();
    }

    {
        ScopedCpuTimer timer(m_performance, "Hierarchy");
        m_hierarchy.update();
    }

    {
        ScopedCpuTimer timer(m_performance, "Components");
        m_components.update();
    }

    {
        ScopedCpuTimer timer(m_performance, "Transforms");
        m_transforms.update();
    }

    {
        ScopedCpuTimer timer(m_performance, "Cameras");
        m_cameras.update();
    }

    {
        ScopedCpuTimer timer(m_performance, "Graphics");
        m_graphics.update();
    }
}

void Engine::render()
{
    ScopedCpuTimer timer(m_performance, "Engine Render");

    m_graphics.render();
}

bool Engine::start_frame()
{
    m_performance.begin_frame();

    bool ready = true;

    {
        ScopedCpuTimer timer(m_performance, "Engine Start Frame");

        {
            ScopedCpuTimer timer(m_performance, "Poll Events");
            m_platform.window().poll_events();
        }

        {
            ScopedCpuTimer timer(m_performance, "Graphics Start Frame");
            m_graphics.start_frame();
        }

        if (m_graphics.needs_swapchain_recreation())
        {
            const uint32_t width = m_platform.window().width();
            const uint32_t height = m_platform.window().height();

            if (width != 0 && height != 0)
            {
                ScopedCpuTimer timer(m_performance, "Swapchain Recreation");

                m_graphics.recreate_swapchain(width, height);
            }

            ready = false;
        }
    }

    if (!ready)
        m_performance.cancel_frame();

    return ready;
}

void Engine::end_frame()
{
    m_graphics.end_frame();

    m_performance.end_frame();
}

void Engine::wait()
{
    m_graphics.wait();
}

bool Engine::should_close() const 
{
    return m_platform.window().should_close();
}

EventSystem& Engine::events()
{
    return m_events;
}

EntitySystem& Engine::entities()
{
    return m_entities;
}

HierarchySystem& Engine::hierarchy()
{
    return m_hierarchy;
}

ComponentSystem& Engine::components()
{
    return m_components;
}

TransformSystem& Engine::transforms()
{
    return m_transforms;
}

CameraSystem& Engine::cameras()
{
    return m_cameras;
}

PlatformSystem& Engine::platform()
{
    return m_platform;
}

GraphicsSystem& Engine::graphics()
{
    return m_graphics;
}

PerformanceSystem& Engine::performance()
{
    return m_performance;
}

} // namespace omega