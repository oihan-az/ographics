#ifndef OGFX_SEMAPHORE_HPP
#define OGFX_SEMAPHORE_HPP

#include <memory>

namespace ogfx
{

class Device;

class Semaphore
{
  public:
    explicit Semaphore(const Device& device);
    ~Semaphore();

    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

    Semaphore(Semaphore&&) noexcept;
    Semaphore& operator=(Semaphore&&) noexcept;

  private:
    friend class Queue;
    friend class Swapchain;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_SEMAPHORE_HPP