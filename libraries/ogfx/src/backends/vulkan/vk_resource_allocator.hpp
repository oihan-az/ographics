#ifndef OGFX_VK_RESOURCE_ALLOCATOR_HPP
#define OGFX_VK_RESOURCE_ALLOCATOR_HPP

#include <ogfx/resource_allocator.hpp>

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <vector>

namespace ogfx
{

struct ResourceAllocator::Impl
{
    VkDevice m_device = VK_NULL_HANDLE;
    uint32_t m_max_sets_per_pool = 0;

    std::vector<VkDescriptorPool> m_descriptor_pools;
    std::unordered_map<VkDescriptorSet, VkDescriptorPool> m_set_pools;
};

} // namespace ogfx

#endif