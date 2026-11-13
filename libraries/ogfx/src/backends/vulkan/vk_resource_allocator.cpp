#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_resource_allocator.hpp"
#include "vk_device.hpp"
#include "vk_resource_set_layout.hpp"
#include "vk_resource_set.hpp"
#include "vk_resource_type.hpp"

#include <array>
#include <string>
#include <map>

namespace ogfx
{

namespace
{

struct DescriptorPoolSizes
{
    std::map<VkDescriptorType, uint32_t> counts;

    bool add(VkDescriptorType type, uint32_t count)
    {
        if (count == 0)
            return true;

        uint32_t& current = counts[type];

        if (count > std::numeric_limits<uint32_t>::max() - current)
            return false;

        current += count;
        return true;
    }

    std::vector<VkDescriptorPoolSize> get_sizes() const
    {
        std::vector<VkDescriptorPoolSize> result;
        result.reserve(counts.size());

        for (const auto& [type, count] : counts)
        {
            result.push_back({.type = type, .descriptorCount = count});
        }

        return result;
    }
};

} // namespace

ResourceAllocator::ResourceAllocator(const Device& device, uint32_t max_sets_per_pool)
    : m_impl(std::make_unique<Impl>()), m_max_sets_per_pool(max_sets_per_pool)
{
    m_impl->m_device = device.m_impl->m_device;

    OGFX_LOG("Resource allocator created");
    OGFX_LOG("  Max sets per pool: " + std::to_string(max_sets_per_pool));
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

    const std::vector<VkDescriptorPoolSize> pool_sizes = sizes.get_sizes();

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
    OGFX_LOG("  Max sets: " + std::to_string(desc.max_sets));
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

    auto try_allocate = [&]() -> bool
    {
        for (VkDescriptorPool pool : m_impl->m_descriptor_pools)
        {
            OGFX_LOG("Trying descriptor pool index " + std::to_string(i));

            const VkDescriptorSetAllocateInfo allocate_info{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                                            .pNext = nullptr,
                                                            .descriptorPool = pool,
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

            return true;
        }

        return false;
    };

    // Try all existing pools
    bool allocated = try_allocate();
    if (allocated)
        return;

    OGFX_LOG("No existing pool could allocate the descriptor set");

    // Create a pool sized for this layout and the configured number of sets
    DescriptorPoolDesc pool_desc;
    pool_desc.max_sets = m_max_sets_per_pool;

    DescriptorPoolSizes required_sizes;
    for (const ResourceBindingDesc& binding : layout.m_desc.bindings)
    {
        const VkDescriptorType type = to_vk_descriptor_type(binding.type);

        const uint64_t descriptor_count = static_cast<uint64_t>(binding.array_count) * pool_desc.max_sets;

        if (descriptor_count > std::numeric_limits<uint32_t>::max() ||
            !required_sizes.add(type, static_cast<uint32_t>(descriptor_count)))
        {
            throw std::runtime_error("Descriptor pool size exceeds supported capacity");
        }
    }

    for (const auto& [type, count] : required_sizes.counts)
    {
        ogfx::DescriptorPoolSize pool_size;
        pool_size.type = from_vk_descriptor_type(type);
        pool_size.descriptor_count = count;

        pool_desc.sizes.push_back(pool_size);
    }

    create_pool(pool_desc);

    OGFX_LOG("Automatically created descriptor pool:");
    OGFX_LOG("  Max sets: " + std::to_string(pool_desc.max_sets));
    OGFX_LOG("  Pool count: " + std::to_string(m_impl->m_descriptor_pools.size()));

    allocated = try_allocate();

    if (!allocated)
    {
        OGFX_LOG("Failed to allocate descriptor set from new pool");
        throw std::runtime_error("Failed to allocate descriptor set from new pool");
    }
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