#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/binding_commands.hpp>

#include <vulkan/vulkan.h>

#include "../vk_command_buffer.hpp"
#include "../vk_pipeline.hpp"
#include "../vk_pipeline_layout.hpp"
#include "../vk_pipeline_type.hpp"
#include "../vk_buffer.hpp"
#include "../vk_index_type.hpp"
#include "../vk_resource_set.hpp"

namespace ogfx
{

void CommandBuffer::bind_pipeline(const PipelineBindInfo& bind_info)
{
    const PipelineType type = bind_info.pipeline->type();
    const VkPipelineBindPoint bind_point = to_vk_pipeline_type(type);

    OGFX_LOG("Binding " + to_string_enum(type) + " pipeline");

    vkCmdBindPipeline(m_impl->m_command_buffer, bind_point, bind_info.pipeline->m_impl->m_pipeline);

    OGFX_LOG(to_string_enum(type) + " pipeline bound");
}

void CommandBuffer::bind_vertex_buffer(const VertexBufferBindInfo& bind_info)
{
    OGFX_LOG("Binding vertex buffer:");
    OGFX_LOG("  Binding: " + std::to_string(bind_info.binding));
    OGFX_LOG("  Offset: " + std::to_string(bind_info.offset));

    VkBuffer vertex_buffer = bind_info.buffer->m_impl->m_buffer;
    VkDeviceSize buffer_offset = static_cast<VkDeviceSize>(bind_info.offset);

    vkCmdBindVertexBuffers(m_impl->m_command_buffer, bind_info.binding, 1, &vertex_buffer, &buffer_offset);

    OGFX_LOG("Vertex buffer bound");
}

void CommandBuffer::bind_index_buffer(const IndexBufferBindInfo& bind_info)
{
    OGFX_LOG("Binding index buffer:");
    OGFX_LOG("  Offset: " + std::to_string(bind_info.offset));
    OGFX_LOG("  Index type: " + to_string_enum(bind_info.index_type));

    VkBuffer index_buffer = bind_info.buffer->m_impl->m_buffer;
    VkDeviceSize buffer_offset = static_cast<VkDeviceSize>(bind_info.offset);
    VkIndexType index_type = to_vk_index_type(bind_info.index_type);

    vkCmdBindIndexBuffer(m_impl->m_command_buffer, index_buffer, buffer_offset, index_type);

    OGFX_LOG("Index buffer bound");
}

void CommandBuffer::bind_resource_set(const ResourceSetBindInfo& info)
{
    OGFX_LOG("Binding resource set:");
    OGFX_LOG("  Set index: " + std::to_string(info.set_index));
    OGFX_LOG("  Pipeline bind point: " + to_string_enum(info.pipeline_type));

    const VkDescriptorSet descriptor_set = info.resource_set->m_impl->m_descriptor_set;
    const VkPipelineBindPoint bind_point = to_vk_pipeline_type(info.pipeline_type);

    vkCmdBindDescriptorSets(m_impl->m_command_buffer, bind_point, info.layout->m_impl->m_pipeline_layout,
                            info.set_index, 1, &descriptor_set, 0, nullptr);

    OGFX_LOG("Resource set bound");
}

} // namespace ogfx