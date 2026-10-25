#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/inline_constants_commands.hpp>

#include <vulkan/vulkan.h>

#include "../vk_command_buffer.hpp"
#include "../vk_pipeline_layout.hpp"
#include "../vk_shader_stage.hpp"

namespace ogfx
{

void CommandBuffer::set_inline_constants(const InlineConstantsInfo& info)
{
    OGFX_LOG("Setting inline constants:");
    OGFX_LOG("  Stages: " + to_string_bitmask(info.stages));
    OGFX_LOG("  Offset: " + std::to_string(info.offset));
    OGFX_LOG("  Size: " + std::to_string(info.size));

    vkCmdPushConstants(m_impl->m_command_buffer, info.layout->m_impl->m_pipeline_layout,
                       to_vk_shader_stage_flags(info.stages), info.offset, info.size, info.data);

    OGFX_LOG("Inline constants set");
}

} // namespace ogfx