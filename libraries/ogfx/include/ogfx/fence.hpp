#ifndef OGFX_FENCE_HPP
#define OGFX_FENCE_HPP

#include <memory>

namespace ogfx
{

class Device;

class Fence
{
  public:
    explicit Fence(const Device& device, bool signaled = false);
    ~Fence();

    Fence(const Fence&) = delete;
    Fence& operator=(const Fence&) = delete;

    Fence(Fence&&) noexcept;
    Fence& operator=(Fence&&) noexcept;

    void wait();
    void reset();

  private:
    friend class Queue;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_FENCE_HPP