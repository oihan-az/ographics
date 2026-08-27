#include "vk_present_result.hpp"

#include <stdexcept>

namespace ogfx
{

VkResult to_vk_present_result(PresentResult result)
{
    switch (result)
    {
    case PresentResult::Success:
        return VK_SUCCESS;

    case PresentResult::Suboptimal:
        return VK_SUBOPTIMAL_KHR;

    case PresentResult::OutOfDate:
        return VK_ERROR_OUT_OF_DATE_KHR;

    default:
        return VK_ERROR_UNKNOWN;
    }
}

PresentResult from_vk_present_result(VkResult result)
{
    switch (result)
    {
    case VK_SUCCESS:
        return PresentResult::Success;

    case VK_SUBOPTIMAL_KHR:
        return PresentResult::Suboptimal;

    case VK_ERROR_OUT_OF_DATE_KHR:
        return PresentResult::OutOfDate;

    default:
        throw std::runtime_error("Unknown Vulkan present result.");
    }
}

} // namespace ogfx