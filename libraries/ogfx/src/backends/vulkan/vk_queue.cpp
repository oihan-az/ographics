#include <debug.hpp>

#include "vk_queue.hpp"
#include "vk_command_buffer.hpp"
#include "vk_semaphore.hpp"
#include "vk_fence.hpp"
#include "vk_pipeline_stage.hpp"
#include "vk_swapchain.hpp"
#include "vk_present_result.hpp"

#include <vector>

namespace ogfx
{

Queue::Queue() : m_impl(std::make_unique<Impl>()) {}

Queue::~Queue() {}

Queue::Queue(Queue&&) noexcept = default;

Queue& Queue::operator=(Queue&&) noexcept = default;

void Queue::submit(const SubmitDesc& desc)
{
    OGFX_LOG("Submitting " + std::to_string(desc.command_buffers.size()) +
             " command buffer(s) to Vulkan queue (family " + std::to_string(m_impl->m_family_index) + ")");

    std::vector<VkCommandBuffer> command_buffers;
    command_buffers.reserve(desc.command_buffers.size());
    for (size_t i = 0; i < desc.command_buffers.size(); ++i)
    {
        const CommandBuffer* command_buffer = desc.command_buffers[i];

        command_buffers.push_back(command_buffer->m_impl->m_command_buffer);
    }

    OGFX_LOG("Waiting on " + std::to_string(desc.wait_semaphores.size()) + " semaphore(s)");

    std::vector<VkSemaphore> wait_semaphores;
    std::vector<VkPipelineStageFlags> wait_stages;
    wait_semaphores.reserve(desc.wait_semaphores.size());
    wait_stages.reserve(desc.wait_semaphores.size());

    for (size_t i = 0; i < desc.wait_semaphores.size(); ++i)
    {
        const SubmitWait& wait = desc.wait_semaphores[i];

        wait_semaphores.push_back(wait.semaphore->m_impl->m_semaphore);

        wait_stages.push_back(to_vk_pipeline_stage(wait.stage));

        OGFX_LOG("  Wait semaphore [" + std::to_string(i) + "] at stages: " + to_string(wait.stage));
    }

    OGFX_LOG("Signaling " + std::to_string(desc.signal_semaphores.size()) + " semaphore(s)");

    std::vector<VkSemaphore> signal_semaphores;
    signal_semaphores.reserve(desc.signal_semaphores.size());

    for (size_t i = 0; i < desc.signal_semaphores.size(); ++i)
    {
        const Semaphore* semaphore = desc.signal_semaphores[i];

        signal_semaphores.push_back(semaphore->m_impl->m_semaphore);
    }

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submit_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());

    submit_info.pWaitSemaphores = wait_semaphores.data();

    submit_info.pWaitDstStageMask = wait_stages.data();

    submit_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

    submit_info.pCommandBuffers = command_buffers.data();

    submit_info.signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size());

    submit_info.pSignalSemaphores = signal_semaphores.data();

    VkFence fence = VK_NULL_HANDLE;

    if (desc.fence)
    {
        fence = desc.fence->m_impl->m_fence;

        OGFX_LOG("Signaling fence on submission");
    }
    else
    {
        OGFX_LOG("No fence associated with submission");
    }

    OGFX_LOG("Submitting Vulkan queue");

    if (vkQueueSubmit(m_impl->m_queue, 1, &submit_info, fence) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to submit Vulkan command buffers");
        throw std::runtime_error("Failed to submit Vulkan command buffers.");
    }

    OGFX_LOG("Vulkan queue submission completed");
}

PresentResult Queue::present(const Swapchain& swapchain, uint32_t image_index, const Semaphore& wait_semaphore)
{
    OGFX_LOG("Presenting swapchain image [" + std::to_string(image_index) + "] on Vulkan queue (family " +
             std::to_string(m_impl->m_family_index) + ")");

    OGFX_LOG("  Waiting on render semaphore");

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    VkSemaphore vk_wait_semaphore = wait_semaphore.m_impl->m_semaphore;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &vk_wait_semaphore;

    VkSwapchainKHR vk_swapchain = swapchain.m_impl->m_swapchain;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vk_swapchain;

    present_info.pImageIndices = &image_index;

    const VkResult vk_result = vkQueuePresentKHR(m_impl->m_queue, &present_info);

    const PresentResult result = from_vk_present_result(vk_result);
    OGFX_LOG("Presentation result: " + std::string(to_string(result)));

    return result;
}

bool Queue::valid() const
{
    return m_impl && m_impl->m_queue != VK_NULL_HANDLE;
}

} // namespace ogfx