#include <debug.hpp>

#include "vk_resource_set.hpp"
#include "vk_resource_type.hpp"
#include "vk_resource_set_layout.hpp"
#include "vk_resource_allocator.hpp"
#include "vk_device.hpp"
#include "vk_buffer.hpp"

#include <string>
#include <utility>

namespace ogfx
{

ResourceSet::ResourceSet(const Device&, const ResourceSetDesc& desc) : m_desc(desc), m_impl(std::make_unique<Impl>()) {}

ResourceSet::~ResourceSet() = default;

ResourceSet::ResourceSet(ResourceSet&&) noexcept = default;

ResourceSet& ResourceSet::operator=(ResourceSet&&) noexcept = default;

void ResourceSet::allocate(ResourceAllocator& allocator)
{
    allocator.allocate(*this);
}

void ResourceSet::free(ResourceAllocator& allocator)
{
    allocator.free(*this);
}

void ResourceSet::write(const ResourceSetWriteInfo& info)
{
    OGFX_LOG("Writing descriptors:");
    OGFX_LOG("  Buffer binding count: " + std::to_string(info.buffers.size()));

    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkWriteDescriptorSet> writes;

    buffer_infos.reserve(info.buffers.size());
    writes.reserve(info.buffers.size());

    for (const BufferBinding& binding : info.buffers)
    {
        OGFX_LOG("  Buffer binding:");
        OGFX_LOG("    Binding: " + std::to_string(binding.binding));
        OGFX_LOG("    Offset: " + std::to_string(binding.offset));
        OGFX_LOG("    Size: " + std::to_string(binding.size));

        const VkDescriptorType descriptor_type = to_vk_descriptor_type(m_desc.layout->m_desc.bindings[binding.binding].type);

        if (descriptor_type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
            descriptor_type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
        {
            OGFX_LOG("Binding is not a buffer descriptor");
            return;
        }

        const VkDeviceSize range = binding.size == 0 ? VK_WHOLE_SIZE : static_cast<VkDeviceSize>(binding.size);

        buffer_infos.push_back({.buffer = binding.buffer->m_impl->m_buffer,
                                .offset = static_cast<VkDeviceSize>(binding.offset),
                                .range = range});

        VkWriteDescriptorSet write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                   .pNext = nullptr,
                                   .dstSet = m_impl->m_descriptor_set,
                                   .dstBinding = binding.binding,
                                   .dstArrayElement = 0,
                                   .descriptorCount = 1,
                                   .descriptorType = descriptor_type,
                                   .pImageInfo = nullptr,
                                   .pBufferInfo = &buffer_infos.back(),
                                   .pTexelBufferView = nullptr};

        writes.push_back(write);
    }

    OGFX_LOG("Updating resources:");
    OGFX_LOG("  Write count: " + std::to_string(writes.size()));

    vkUpdateDescriptorSets(m_impl->m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

    OGFX_LOG("Resources updated");
}

} // namespace ogfx