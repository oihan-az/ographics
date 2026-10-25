#include <debug.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/draw_commands.hpp>

#include <vulkan/vulkan.h>

#include "../vk_command_buffer.hpp"
#include "../vk_buffer.hpp"

namespace ogfx
{

void CommandBuffer::draw(const DrawInfo& draw_info)
{
    OGFX_LOG("Drawing:");
    OGFX_LOG("  Vertex count: " + std::to_string(draw_info.vertex_count));
    OGFX_LOG("  Instance count: " + std::to_string(draw_info.instance_count));
    OGFX_LOG("  First vertex: " + std::to_string(draw_info.first_vertex));
    OGFX_LOG("  First instance: " + std::to_string(draw_info.first_instance));

    vkCmdDraw(m_impl->m_command_buffer, draw_info.vertex_count, draw_info.instance_count, draw_info.first_vertex,
              draw_info.first_instance);

    OGFX_LOG("Draw recorded");
}

void CommandBuffer::draw_indexed(const DrawIndexedInfo& draw_info)
{
    OGFX_LOG("Drawing indexed:");
    OGFX_LOG("  Index count: " + std::to_string(draw_info.index_count));
    OGFX_LOG("  Instance count: " + std::to_string(draw_info.instance_count));
    OGFX_LOG("  First index: " + std::to_string(draw_info.first_index));
    OGFX_LOG("  Vertex offset: " + std::to_string(draw_info.vertex_offset));
    OGFX_LOG("  First instance: " + std::to_string(draw_info.first_instance));

    vkCmdDrawIndexed(m_impl->m_command_buffer, draw_info.index_count, draw_info.instance_count, draw_info.first_index,
                     draw_info.vertex_offset, draw_info.first_instance);

    OGFX_LOG("Indexed draw recorded");
}

void CommandBuffer::draw_indexed_indirect(const DrawIndexedIndirectInfo& draw_info)
{
    OGFX_LOG("Drawing indexed indirect:");
    OGFX_LOG("  Offset: " + std::to_string(draw_info.offset));
    OGFX_LOG("  Draw count: " + std::to_string(draw_info.draw_count));
    OGFX_LOG("  Stride: " + std::to_string(draw_info.stride));

    VkBuffer buffer = draw_info.buffer->m_impl->m_buffer;
    VkDeviceSize offset = static_cast<VkDeviceSize>(draw_info.offset);

    vkCmdDrawIndexedIndirect(m_impl->m_command_buffer, buffer, offset, draw_info.draw_count, draw_info.stride);

    OGFX_LOG("Indexed indirect draw recorded");
}

} // namespace ogfx