#ifndef OGFX_SURFACE_HPP
#define OGFX_SURFACE_HPP

#include <memory>

namespace ogfx
{

class Instance;

using NativeSurfaceHandle = void*;

class Surface
{
  private:
    struct Impl;

  public:
    explicit Surface(const Instance& instance, NativeSurfaceHandle handle);
    ~Surface();

    Surface(const Surface&) = delete;
    Surface& operator=(const Surface&) = delete;

    Surface(Surface&&) noexcept;
    Surface& operator=(Surface&&) noexcept;

  private:
    friend class PhysicalDevice;
    friend class Swapchain;

    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_SURFACE_HPP