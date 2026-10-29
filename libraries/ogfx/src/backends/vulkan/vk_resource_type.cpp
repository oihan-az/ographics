#include "vk_resource_type.hpp"

namespace ogfx
{

VkDescriptorType to_vk_descriptor_type(ResourceType type)
{
    switch (type)
    {
    case ResourceType::UniformBuffer:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    case ResourceType::StorageBuffer:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    case ResourceType::SampledTexture:
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

    case ResourceType::StorageTexture:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    case ResourceType::Sampler:
        return VK_DESCRIPTOR_TYPE_SAMPLER;

    case ResourceType::AccelerationStructure:
        return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

    case ResourceType::Undefined:
    default:
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}

ResourceType from_vk_descriptor_type(VkDescriptorType type)
{
    switch (type)
    {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return ResourceType::UniformBuffer;

    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        return ResourceType::StorageBuffer;

    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        return ResourceType::SampledTexture;

    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        return ResourceType::StorageTexture;

    case VK_DESCRIPTOR_TYPE_SAMPLER:
        return ResourceType::Sampler;

    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
        return ResourceType::AccelerationStructure;

    default:
        return ResourceType::Undefined;
    }
}

} // namespace ogfx
