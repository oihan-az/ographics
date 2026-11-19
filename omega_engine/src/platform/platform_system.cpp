#include <omega/platform/platform_system.hpp>

namespace omega
{

PlatformSystem::PlatformSystem(const oplatform::WindowDesc& window_desc) : m_runtime(), m_window(window_desc) {}

oplatform::Runtime& PlatformSystem::runtime()
{
    return m_runtime;
}

const oplatform::Runtime& PlatformSystem::runtime() const
{
    return m_runtime;
}

oplatform::Window& PlatformSystem::window()
{
    return m_window;
}

const oplatform::Window& PlatformSystem::window() const
{
    return m_window;
}

} // namespace omega