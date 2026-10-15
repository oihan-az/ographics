#include "vk_multisample.hpp"

namespace ogfx
{

VkSampleCountFlagBits to_vk_sample_count(SampleCount count)
{
    switch (count)
    {
    case SampleCount::Count1:
        return VK_SAMPLE_COUNT_1_BIT;
    case SampleCount::Count2:
        return VK_SAMPLE_COUNT_2_BIT;
    case SampleCount::Count4:
        return VK_SAMPLE_COUNT_4_BIT;
    case SampleCount::Count8:
        return VK_SAMPLE_COUNT_8_BIT;
    case SampleCount::Count16:
        return VK_SAMPLE_COUNT_16_BIT;
    case SampleCount::Count32:
        return VK_SAMPLE_COUNT_32_BIT;
    case SampleCount::Count64:
        return VK_SAMPLE_COUNT_64_BIT;
    case SampleCount::Undefined:
        return static_cast<VkSampleCountFlagBits>(0);
    }

    return static_cast<VkSampleCountFlagBits>(0);
}

SampleCount from_vk_sample_count(VkSampleCountFlagBits count)
{
    switch (count)
    {
    case VK_SAMPLE_COUNT_1_BIT:
        return SampleCount::Count1;
    case VK_SAMPLE_COUNT_2_BIT:
        return SampleCount::Count2;
    case VK_SAMPLE_COUNT_4_BIT:
        return SampleCount::Count4;
    case VK_SAMPLE_COUNT_8_BIT:
        return SampleCount::Count8;
    case VK_SAMPLE_COUNT_16_BIT:
        return SampleCount::Count16;
    case VK_SAMPLE_COUNT_32_BIT:
        return SampleCount::Count32;
    case VK_SAMPLE_COUNT_64_BIT:
        return SampleCount::Count64;
    default:
        return SampleCount::Undefined;
    }
}

} // namespace ogfx