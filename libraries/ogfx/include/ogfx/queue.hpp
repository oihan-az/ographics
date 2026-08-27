#ifndef OGFX_QUEUE_HPP
#define OGFX_QUEUE_HPP

#include <ogfx/pipeline_stage.hpp>
#include <ogfx/present_result.hpp>

#include <memory>
#include <vector>

namespace ogfx
{

class CommandBuffer;
class Semaphore;
class Fence;
class Swapchain;

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
    PresentResult present(const Swapchain& swapchain, uint32_t image_index, const Semaphore& wait_semaphore);

    [[nodiscard]] bool valid() const;

  private:
    friend class Device;
    friend class CommandPool;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_QUEUE_HPP