#ifndef OGFX_VK_MULTISAMPLE_HPP
#define OGFX_VK_MULTISAMPLE_HPP

#include <ogfx/multisample.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkSampleCountFlagBits to_vk_sample_count(SampleCount count);
SampleCount from_vk_sample_count(VkSampleCountFlagBits count);

} // namespace ogfx

#endif // OGFX_VK_MULTISAMPLE_HPP