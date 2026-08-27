#ifndef OGFX_COMMAND_POOL_HPP
#define OGFX_COMMAND_POOL_HPP

#include <cstdint>
#include <memory>

namespace ogfx
{

class Device;
class Queue;

struct CommandPoolDesc
{
    bool transient = false;
    bool resettable = true;
};

class CommandPool
{
  public:
    CommandPool(const Device& device, const Queue& queue, const CommandPoolDesc& desc);

    ~CommandPool();

    CommandPool(const CommandPool&) = delete;
    CommandPool& operator=(const CommandPool&) = delete;

    CommandPool(CommandPool&&) noexcept;
    CommandPool& operator=(CommandPool&&) noexcept;

  private:
    friend class CommandBuffer;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_COMMAND_POOL_HPP