#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_resource_set.hpp"
#include "vk_resource_type.hpp"
#include "vk_resource_set_layout.hpp"
#include "vk_resource_allocator.hpp"
#include "vk_device.hpp"
#include "vk_buffer.hpp"
#include "vk_image_view.hpp"
#include "vk_image_state.hpp"
#include "vk_sampler.hpp"

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
    OGFX_LOG("  Texture binding count: " + std::to_string(info.textures.size()));
    OGFX_LOG("  Sampler binding count: " + std::to_string(info.samplers.size()));

    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkDescriptorImageInfo> texture_infos;
    std::vector<VkDescriptorImageInfo> sampler_infos;
    std::vector<VkWriteDescriptorSet> writes;

    buffer_infos.reserve(info.buffers.size());
    texture_infos.reserve(info.textures.size());
    sampler_infos.reserve(info.samplers.size());
    writes.reserve(info.buffers.size() + info.textures.size() + info.samplers.size());

    for (const BufferBinding& binding : info.buffers)
    {
        OGFX_LOG("  Buffer binding:");
        OGFX_LOG("    Binding: " + std::to_string(binding.binding));
        OGFX_LOG("    Offset: " + std::to_string(binding.offset));
        OGFX_LOG("    Size: " + std::to_string(binding.size));

        const VkDescriptorType descriptor_type = to_vk_descriptor_type(m_desc.layout->m_desc.bindings[binding.binding].type);

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

    // Sampled image descriptors
    for (const ImageBinding& binding : info.textures)
    {
        OGFX_LOG("  Image binding:");
        OGFX_LOG("    Binding: " + std::to_string(binding.binding));
        OGFX_LOG("    State: " + to_string_enum(binding.image_state));

        const VkDescriptorType descriptor_type = to_vk_descriptor_type(m_desc.layout->m_desc.bindings[binding.binding].type);

        texture_infos.push_back({.sampler = VK_NULL_HANDLE,
                                 .imageView = binding.image_view->m_impl->m_image_view,
                                 .imageLayout = to_vk_image_state(binding.image_state)});

        writes.push_back({.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                          .pNext = nullptr,
                          .dstSet = m_impl->m_descriptor_set,
                          .dstBinding = binding.binding,
                          .dstArrayElement = 0,
                          .descriptorCount = 1,
                          .descriptorType = descriptor_type,
                          .pImageInfo = &texture_infos.back(),
                          .pBufferInfo = nullptr,
                          .pTexelBufferView = nullptr});
    }

    // Sampler descriptors
    for (const SamplerBinding& binding : info.samplers)
    {
        OGFX_LOG("  Sampler binding:");
        OGFX_LOG("    Binding: " + std::to_string(binding.binding));

        const VkDescriptorType descriptor_type = to_vk_descriptor_type(m_desc.layout->m_desc.bindings[binding.binding].type);

        sampler_infos.push_back({.sampler = binding.sampler->m_impl->m_sampler,
                                 .imageView = VK_NULL_HANDLE,
                                 .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED});

        writes.push_back({.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                          .pNext = nullptr,
                          .dstSet = m_impl->m_descriptor_set,
                          .dstBinding = binding.binding,
                          .dstArrayElement = 0,
                          .descriptorCount = 1,
                          .descriptorType = descriptor_type,
                          .pImageInfo = &sampler_infos.back(),
                          .pBufferInfo = nullptr,
                          .pTexelBufferView = nullptr});
    }

    OGFX_LOG("Updating resources:");
    OGFX_LOG("  Write count: " + std::to_string(writes.size()));

    vkUpdateDescriptorSets(m_impl->m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

    OGFX_LOG("Resources updated");
}

} // namespace ogfx