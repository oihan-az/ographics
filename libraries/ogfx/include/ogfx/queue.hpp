#ifndef OGFX_QUEUE_HPP
#define OGFX_QUEUE_HPP

#include <ogfx/pipeline_stage.hpp>

#include <memory>
#include <vector>

namespace ogfx
{

class CommandBuffer;
class Semaphore;
class Fence;

struct SubmitWait
{
    const Semaphore* semaphore = nullptr;
    PipelineStage stage;
};

struct SubmitDesc
{
    std::vector<const CommandBuffer*> command_buffers;

    std::vector<SubmitWait> wait_semaphores;
    std::vector<const Semaphore*> signal_semaphores;

    Fence* fence = nullptr;
};

class Queue
{
  public:
    explicit Queue();
    ~Queue();

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    Queue(Queue&&) noexcept;
    Queue& operator=(Queue&&) noexcept;

    void submit(const SubmitDesc& desc);

    [[nodiscard]] bool valid() const;

  private:
    friend class Device;
    friend class CommandPool;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_QUEUE_HPP