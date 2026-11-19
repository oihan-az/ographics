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
      m_graphics(desc.application_name, desc.max_frames_in_flight, m_platform, m_events)
{}

void Engine::initialize()
{
    m_graphics.initialize();
}

void Engine::update()
{
    m_events.swap_all_buffers();

    m_entities.update();
    m_hierarchy.update();
    m_components.update();
    m_transforms.update();
    m_cameras.update();
    m_graphics.update();
}

void Engine::render()
{
    m_graphics.render();
}

bool Engine::start_frame()
{
    m_platform.window().poll_events();

    m_graphics.start_frame();

    if (m_graphics.needs_swapchain_recreation())
    {
        const uint32_t width = m_platform.window().width();
        const uint32_t height = m_platform.window().height();
        if (width == 0 || height == 0)
            return false;

        m_graphics.recreate_swapchain(width, height);
        return false;
    }

    return true;
}

void Engine::end_frame()
{
    m_graphics.end_frame();
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

} // namespace omega