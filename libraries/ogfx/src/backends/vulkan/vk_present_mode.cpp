#include "vk_present_mode.hpp"

namespace ogfx
{

VkPresentModeKHR to_vk_present_mode(PresentMode mode)
{
    switch (mode)
    {
    case PresentMode::Immediate:
        return VK_PRESENT_MODE_IMMEDIATE_KHR;

    case PresentMode::FIFO:
        return VK_PRESENT_MODE_FIFO_KHR;

    case PresentMode::Mailbox:
        return VK_PRESENT_MODE_MAILBOX_KHR;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

PresentMode from_vk_present_mode(VkPresentModeKHR mode)
{
    switch (mode)
    {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:
        return PresentMode::Immediate;

    case VK_PRESENT_MODE_FIFO_KHR:
        return PresentMode::FIFO;

    case VK_PRESENT_MODE_MAILBOX_KHR:
        return PresentMode::Mailbox;

    default:
        return PresentMode::FIFO;
    }
}

} // namespace ogfx