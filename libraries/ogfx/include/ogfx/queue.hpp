#ifndef OGFX_QUEUE_HPP
#define OGFX_QUEUE_HPP

#include <memory>

namespace ogfx
{

class Queue
{
  public:
    explicit Queue();
    ~Queue();

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    Queue(Queue&&) noexcept;
    Queue& operator=(Queue&&) noexcept;

    [[nodiscard]] bool valid() const;

  private:
    friend class Device;
    friend class CommandPool;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_QUEUE_HPP