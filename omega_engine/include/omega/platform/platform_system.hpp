#ifndef OMEGA_PLATFORM_SYSTEM_HPP
#define OMEGA_PLATFORM_SYSTEM_HPP

#include <oplatform/runtime.hpp>
#include <oplatform/window.hpp>

namespace omega
{

class PlatformSystem
{
  public:
    PlatformSystem(const oplatform::WindowDesc& window_desc);

    [[nodiscard]] oplatform::Runtime& runtime();
    [[nodiscard]] const oplatform::Runtime& runtime() const;

    [[nodiscard]] oplatform::Window& window();
    [[nodiscard]] const oplatform::Window& window() const;

  private:
    oplatform::Runtime m_runtime;
    oplatform::Window m_window;
};

} // namespace omega

#endif // OMEGA_PLATFORM_SYSTEM_HPP