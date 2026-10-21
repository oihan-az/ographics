#ifndef OGFX_VK_SAMPLER_HPP
#define OGFX_VK_SAMPLER_HPP

#include <ogfx/sampler.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkFilter to_vk_filter(Filter filter);
Filter from_vk_filter(VkFilter filter);

VkSamplerMipmapMode to_vk_mipmap_filter(MipmapFilter filter);
MipmapFilter from_vk_mipmap_filter(VkSamplerMipmapMode mode);

VkSamplerAddressMode to_vk_address_mode(AddressMode mode);
AddressMode from_vk_address_mode(VkSamplerAddressMode mode);

VkBorderColor to_vk_border_color(BorderColor color);
BorderColor from_vk_border_color(VkBorderColor color);

struct Sampler::Impl
{
    VkDevice m_device = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_SAMPLER_HPP