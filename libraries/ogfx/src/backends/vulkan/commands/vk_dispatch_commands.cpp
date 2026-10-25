#include <debug.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/dispatch_commands.hpp>

#include <vulkan/vulkan.h>

#include "../vk_command_buffer.hpp"

namespace ogfx
{

void CommandBuffer::dispatch(const DispatchInfo& dispatch_info)
{
    OGFX_LOG("Dispatching compute shader:");
    OGFX_LOG("  Group count X: " + std::to_string(dispatch_info.group_count_x));
    OGFX_LOG("  Group count Y: " + std::to_string(dispatch_info.group_count_y));
    OGFX_LOG("  Group count Z: " + std::to_string(dispatch_info.group_count_z));

    vkCmdDispatch(m_impl->m_command_buffer, dispatch_info.group_count_x, dispatch_info.group_count_y,
                  dispatch_info.group_count_z);

    OGFX_LOG("Compute shader dispatched");
}

} // namespace ogfx