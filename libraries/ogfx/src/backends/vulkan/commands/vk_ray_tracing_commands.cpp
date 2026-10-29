#include <debug.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/ray_tracing_commands.hpp>

#include <vulkan/vulkan.h>

#include "../vk_command_buffer.hpp"
#include "../vk_device_functions.hpp"
#include "../vk_buffer.hpp"
#include "../vk_ray_tracing_pipeline.hpp"

namespace ogfx
{

void CommandBuffer::trace_rays(const TraceRaysInfo& info)
{
    if (!m_impl || info.width == 0 || info.height == 0 || info.depth == 0)
    {
        OGFX_LOG_ERROR("Cannot record trace rays command: invalid launch dimensions");
        return;
    }

    OGFX_LOG("Recording trace rays dispatch:");
    OGFX_LOG("  Launch Size: " + std::to_string(info.width) + "x" + std::to_string(info.height) + "x" +
             std::to_string(info.depth));

    uint64_t base_address = info.sbt_buffer->device_address();
    uint32_t handle_size = info.pipeline->m_impl->m_handle_size;

    uint32_t raygen_index = 0;
    uint32_t miss_index = 1;
    uint32_t hit_index = 1 + info.pipeline->m_impl->m_miss_count;

    uint32_t miss_count = info.pipeline->m_impl->m_miss_count;
    uint32_t hit_count = info.pipeline->m_impl->m_hit_count;

    VkStridedDeviceAddressRegionKHR raygen_region{};
    raygen_region.deviceAddress = base_address + (static_cast<uint64_t>(raygen_index) * handle_size);
    raygen_region.stride = handle_size;
    raygen_region.size = handle_size;

    VkStridedDeviceAddressRegionKHR miss_region{};
    miss_region.deviceAddress = base_address + (static_cast<uint64_t>(miss_index) * handle_size);
    miss_region.stride = miss_count > 0 ? handle_size : 0;
    miss_region.size = handle_size * miss_count;

    VkStridedDeviceAddressRegionKHR hit_region{};
    hit_region.deviceAddress = base_address + (static_cast<uint64_t>(hit_index) * handle_size);
    hit_region.stride = hit_count > 0 ? handle_size : 0;
    hit_region.size = handle_size * hit_count;

    VkStridedDeviceAddressRegionKHR callable_region{};

    vkCmdTraceRaysKHR(m_impl->m_command_buffer, &raygen_region, &miss_region, &hit_region, &callable_region, info.width,
                      info.height, info.depth);

    OGFX_LOG("Trace rays dispatch recorded");
}


} // namespace ogfx