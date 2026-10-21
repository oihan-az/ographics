#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_sampler.hpp"
#include "vk_device.hpp"

namespace ogfx
{

VkFilter to_vk_filter(Filter filter)
{
    switch (filter)
    {
    case Filter::Nearest:
        return VK_FILTER_NEAREST;

    case Filter::Linear:
        return VK_FILTER_LINEAR;

    case Filter::Undefined:
    default:
        return VK_FILTER_NEAREST;
    }
}

Filter from_vk_filter(VkFilter filter)
{
    switch (filter)
    {
    case VK_FILTER_NEAREST:
        return Filter::Nearest;

    case VK_FILTER_LINEAR:
        return Filter::Linear;

    default:
        return Filter::Undefined;
    }
}

VkSamplerMipmapMode to_vk_mipmap_filter(MipmapFilter filter)
{
    switch (filter)
    {
    case MipmapFilter::Nearest:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;

    case MipmapFilter::Linear:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;

    case MipmapFilter::Undefined:
    default:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }
}

MipmapFilter from_vk_mipmap_filter(VkSamplerMipmapMode mode)
{
    switch (mode)
    {
    case VK_SAMPLER_MIPMAP_MODE_NEAREST:
        return MipmapFilter::Nearest;

    case VK_SAMPLER_MIPMAP_MODE_LINEAR:
        return MipmapFilter::Linear;

    default:
        return MipmapFilter::Undefined;
    }
}

VkSamplerAddressMode to_vk_address_mode(AddressMode mode)
{
    switch (mode)
    {
    case AddressMode::Repeat:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;

    case AddressMode::MirroredRepeat:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

    case AddressMode::ClampToEdge:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    case AddressMode::ClampToBorder:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

    case AddressMode::Undefined:
    default:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

AddressMode from_vk_address_mode(VkSamplerAddressMode mode)
{
    switch (mode)
    {
    case VK_SAMPLER_ADDRESS_MODE_REPEAT:
        return AddressMode::Repeat;

    case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT:
        return AddressMode::MirroredRepeat;

    case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
        return AddressMode::ClampToEdge;

    case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
        return AddressMode::ClampToBorder;

    default:
        return AddressMode::Undefined;
    }
}

VkBorderColor to_vk_border_color(BorderColor color)
{
    switch (color)
    {
    case BorderColor::TransparentBlack:
        return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

    case BorderColor::OpaqueBlack:
        return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

    case BorderColor::OpaqueWhite:
        return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    case BorderColor::Undefined:
    default:
        return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    }
}

BorderColor from_vk_border_color(VkBorderColor color)
{
    switch (color)
    {
    case VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK:
        return BorderColor::TransparentBlack;

    case VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK:
        return BorderColor::OpaqueBlack;

    case VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE:
        return BorderColor::OpaqueWhite;

    default:
        return BorderColor::Undefined;
    }
}

Sampler::Sampler(const Device& device, const SamplerDesc& desc) : m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;

    OGFX_LOG("Creating sampler:");
    OGFX_LOG("  Min filter: " + to_string_enum(desc.min_filter));
    OGFX_LOG("  Mag filter: " + to_string_enum(desc.mag_filter));
    OGFX_LOG("  Mip filter: " + to_string_enum(desc.mip_filter));
    OGFX_LOG("  Address U: " + to_string_enum(desc.address_u));
    OGFX_LOG("  Address V: " + to_string_enum(desc.address_v));
    OGFX_LOG("  Address W: " + to_string_enum(desc.address_w));
    OGFX_LOG("  Border color: " + to_string_enum(desc.border_color));
    OGFX_LOG("  Mip LOD bias: " + std::to_string(desc.mip_lod_bias));
    OGFX_LOG("  Min LOD: " + std::to_string(desc.min_lod));
    OGFX_LOG("  Max LOD: " + std::to_string(desc.max_lod));
    OGFX_LOG("  Anisotropy enabled: " + std::string(desc.anisotropy_enable ? "true" : "false"));
    OGFX_LOG("  Max anisotropy: " + std::to_string(desc.max_anisotropy));

    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    sampler_info.magFilter = to_vk_filter(desc.mag_filter);
    sampler_info.minFilter = to_vk_filter(desc.min_filter);
    sampler_info.mipmapMode = to_vk_mipmap_filter(desc.mip_filter);

    sampler_info.addressModeU = to_vk_address_mode(desc.address_u);
    sampler_info.addressModeV = to_vk_address_mode(desc.address_v);
    sampler_info.addressModeW = to_vk_address_mode(desc.address_w);

    sampler_info.mipLodBias = desc.mip_lod_bias;

    sampler_info.anisotropyEnable = desc.anisotropy_enable ? VK_TRUE : VK_FALSE;
    sampler_info.maxAnisotropy = desc.max_anisotropy;

    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

    sampler_info.minLod = desc.min_lod;
    sampler_info.maxLod = desc.max_lod;

    sampler_info.borderColor = to_vk_border_color(desc.border_color);
    sampler_info.unnormalizedCoordinates = VK_FALSE;

    const VkResult result = vkCreateSampler(m_impl->m_device, &sampler_info, nullptr, &m_impl->m_sampler);

    if (result != VK_SUCCESS)
    {
        m_impl->m_sampler = VK_NULL_HANDLE;
        OGFX_LOG("Failed to create sampler");
    }
}

Sampler::~Sampler()
{
    if (m_impl && m_impl->m_sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(m_impl->m_device, m_impl->m_sampler, nullptr);

        OGFX_LOG("Sampler destroyed");
    }
}

Sampler::Sampler(Sampler&&) noexcept = default;
Sampler& Sampler::operator=(Sampler&&) noexcept = default;

} // namespace ogfx