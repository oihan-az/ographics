#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_resource_allocator.hpp"
#include "vk_device.hpp"
#include "vk_resource_set_layout.hpp"
#include "vk_resource_set.hpp"
#include "vk_resource_type.hpp"

#include <array>
#include <string>

namespace ogfx
{

namespace
{

constexpr std::array<VkDescriptorType, 5> types = 
{
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 
    VK_DESCRIPTOR_TYPE_SAMPLER
};

struct DescriptorPoolSizes
{
    std::array<uint32_t, 5> counts{};

    bool add(VkDescriptorType type, uint32_t count)
    {
        size_t index = 0;

        switch (type)
        {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            index = 0;
            break;

        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            index = 1;
            break;

        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            index = 2;
            break;

        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            index = 3;
            break;

        case VK_DESCRIPTOR_TYPE_SAMPLER:
            index = 4;
            break;

        default:
            return false;
        }

        counts[index] += count;
        return true;
    }
};

} // namespace

ResourceAllocator::ResourceAllocator(const Device& device)
    : m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;

    OGFX_LOG("Resource allocator created");
}

ResourceAllocator::~ResourceAllocator()
{
    if (m_impl)
    {
        OGFX_LOG("Destroying resource allocator:");
        OGFX_LOG("  Descriptor pool count: " + std::to_string(m_impl->m_descriptor_pools.size()));
        OGFX_LOG("  Allocated set count: " + std::to_string(m_impl->m_set_pools.size()));

        for (VkDescriptorPool pool : m_impl->m_descriptor_pools)
        {
            if (pool != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorPool(m_impl->m_device, pool, nullptr);
            }
        }
    }

    OGFX_LOG("Resource allocator destroyed");
}

ResourceAllocator::ResourceAllocator(ResourceAllocator&&) noexcept = default;

ResourceAllocator& ResourceAllocator::operator=(ResourceAllocator&&) noexcept = default;

void ResourceAllocator::create_pool(const DescriptorPoolDesc& desc)
{
    OGFX_LOG("Creating descriptor pool:");
    OGFX_LOG("  Max sets: " + std::to_string(desc.max_sets));
    OGFX_LOG("  Descriptor type count: " + std::to_string(desc.sizes.size()));

    if (desc.max_sets == 0)
    {
        OGFX_LOG_ERROR("Max sets must be greater than zero");
        throw std::runtime_error("Max sets must be greater than zero");
    }

    OGFX_LOG("Descriptor pool sizes:");

    DescriptorPoolSizes sizes;
    for (const DescriptorPoolSize& size : desc.sizes)
    {
        const VkDescriptorType type = to_vk_descriptor_type(size.type);
        if (!sizes.add(type, size.descriptor_count))
        {
            OGFX_LOG_ERROR("Unsupported descriptor type");
            throw std::runtime_error("Unsupported descriptor type");
        }

        OGFX_LOG("  " + to_string_enum(size.type) + ": " + std::to_string(size.descriptor_count));
    }

    std::vector<VkDescriptorPoolSize> pool_sizes;
    pool_sizes.reserve(types.size());
    for (size_t i = 0; i < types.size(); ++i)
    {
        if (sizes.counts[i] == 0)
        {
            continue;
        }

        pool_sizes.push_back({.type = types[i], .descriptorCount = sizes.counts[i]});
    }

    const VkDescriptorPoolCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                                 .pNext = nullptr,
                                                 .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
                                                 .maxSets = desc.max_sets,
                                                 .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
                                                 .pPoolSizes = pool_sizes.data()};

    VkDescriptorPool pool = VK_NULL_HANDLE;
    const VkResult result = vkCreateDescriptorPool(m_impl->m_device, &create_info, nullptr, &pool);

    if (result != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create descriptor pool");
        throw std::runtime_error("Failed to create descriptor pool");
    }

    m_impl->m_descriptor_pools.push_back(pool);

    OGFX_LOG("Descriptor pool created:");
    OGFX_LOG("  Pool count: " + std::to_string(m_impl->m_descriptor_pools.size()));
}

void ResourceAllocator::allocate(ResourceSet& resource_set)
{
    if (resource_set.m_impl->m_descriptor_set != VK_NULL_HANDLE)
    {
        OGFX_LOG("Resource set is already allocated");
        return;
    }

    const ResourceSetLayout& layout = *resource_set.m_desc.layout;

    OGFX_LOG("Allocating resource set:");
    OGFX_LOG("  Descriptor pool count: " + std::to_string(m_impl->m_descriptor_pools.size()));

    for (size_t i = 0; i < m_impl->m_descriptor_pools.size(); ++i)
    {
        const VkDescriptorPool pool = m_impl->m_descriptor_pools[i];

        OGFX_LOG("Trying descriptor pool index " + std::to_string(i));

        const VkDescriptorSetAllocateInfo allocate_info{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                                        .pNext = nullptr,
                                                        .descriptorPool = m_impl->m_descriptor_pools[i],
                                                        .descriptorSetCount = 1,
                                                        .pSetLayouts = &layout.m_impl->m_descriptor_set_layout};

        VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
        if (vkAllocateDescriptorSets(m_impl->m_device, &allocate_info, &descriptor_set) != VK_SUCCESS)
        {
            OGFX_LOG("Allocation failed for pool index " + std::to_string(i));
            continue;
        }

        resource_set.m_impl->m_descriptor_set = descriptor_set;
        resource_set.m_impl->m_device = m_impl->m_device;

        m_impl->m_set_pools.emplace(descriptor_set, pool);

        OGFX_LOG("Resource set allocated successfully:");
            OGFX_LOG("  Pool index: " + std::to_string(i));

        return;
    }

    OGFX_LOG("No existing pool could allocate the descriptor set");
}

void ResourceAllocator::free(ResourceSet& resource_set)
{
    if (!m_impl || !resource_set.m_impl)
    {
        return;
    }

    OGFX_LOG("Freeing resource set");

    const VkDescriptorSet descriptor_set = resource_set.m_impl->m_descriptor_set;

    if (descriptor_set == VK_NULL_HANDLE)
    {
        return;
    }

    const auto it = m_impl->m_set_pools.find(descriptor_set);

    if (it == m_impl->m_set_pools.end())
    {
        OGFX_LOG("Descriptor set is not owned by this allocator");
        return;
    }

    const VkDescriptorPool pool = it->second;

    const VkResult result = vkFreeDescriptorSets(m_impl->m_device, pool, 1, &descriptor_set);

    if (result != VK_SUCCESS)
    {
        OGFX_LOG("Failed to free descriptor set: " + std::to_string(static_cast<int>(result)));
        return;
    }

    m_impl->m_set_pools.erase(it);

    resource_set.m_impl->m_descriptor_set = VK_NULL_HANDLE;
    resource_set.m_impl->m_device = VK_NULL_HANDLE;

    OGFX_LOG("Resource set freed successfully");
}

} // namespace ogfx