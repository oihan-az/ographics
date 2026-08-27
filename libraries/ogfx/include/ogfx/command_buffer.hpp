#ifndef OGFX_COMMAND_BUFFER_HPP
#define OGFX_COMMAND_BUFFER_HPP

#include <memory>

namespace ogfx
{

class CommandPool;

class CommandBuffer
{
  public:
    explicit CommandBuffer(const CommandPool& command_pool);

    ~CommandBuffer();

    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer& operator=(const CommandBuffer&) = delete;

    CommandBuffer(CommandBuffer&&) noexcept;
    CommandBuffer& operator=(CommandBuffer&&) noexcept;

    void begin();
    void end();

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_COMMAND_BUFFER_HPP