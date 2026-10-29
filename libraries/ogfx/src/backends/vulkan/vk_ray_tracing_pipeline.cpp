#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_ray_tracing_pipeline.hpp"
#include "vk_pipeline.hpp"
#include "vk_device.hpp"
#include "vk_device_functions.hpp"
#include "vk_physical_device.hpp"
#include "vk_pipeline_layout.hpp"
#include "vk_shader.hpp"
#include "vk_shader_stage.hpp"
#include "vk_buffer.hpp"

#include <stdexcept>

namespace ogfx
{

RayTracingPipeline::RayTracingPipeline(const PhysicalDevice& physical_device, const Device& device,
                                       const RayTracingPipelineDesc& desc)
    : Pipeline(device), m_impl(std::make_unique<Impl>())
{
    if (desc.layout == nullptr || !desc.layout->m_impl)
    {
        OGFX_LOG_ERROR("Cannot create ray tracing pipeline: invalid pipeline layout");
        throw std::invalid_argument("Ray tracing pipeline requires a valid pipeline layout.");
    }

    if (desc.raygen_shader == nullptr)
    {
        OGFX_LOG_ERROR("Cannot create ray tracing pipeline: missing mandatory raygen shader");
        throw std::invalid_argument("Ray tracing pipeline requires a valid raygen shader.");
    }

    OGFX_LOG("Creating ray tracing pipeline:");
    OGFX_LOG("  Max recursion depth: " + std::to_string(desc.max_recursion_depth));

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;

    auto add_shader_stage = [&](const Shader* shader) -> uint32_t
    {
        if (shader == nullptr || !shader->m_impl)
        {
            return VK_SHADER_UNUSED_KHR;
        }

        VkPipelineShaderStageCreateInfo stage_info{};
        stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_info.stage = to_vk_shader_stage(shader->m_stage);
        stage_info.module = shader->m_impl->m_shader_module;
        stage_info.pName = shader->m_entry_point.c_str();

        shader_stages.push_back(stage_info);
        return static_cast<uint32_t>(shader_stages.size() - 1);
    };

    uint32_t raygen_index = add_shader_stage(desc.raygen_shader);

    VkRayTracingShaderGroupCreateInfoKHR raygen_group{};
    raygen_group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    raygen_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    raygen_group.generalShader = raygen_index;
    raygen_group.closestHitShader = VK_SHADER_UNUSED_KHR;
    raygen_group.anyHitShader = VK_SHADER_UNUSED_KHR;
    raygen_group.intersectionShader = VK_SHADER_UNUSED_KHR;
    groups.push_back(raygen_group);

    OGFX_LOG("  Ray generation count: " + std::to_string(1));
    OGFX_LOG("    Group [" + std::to_string(groups.size() - 1) + "] Type: General");
    OGFX_LOG("      Ray generation Shader Index: " + std::to_string(raygen_index));

    OGFX_LOG("  Miss shaders count: " + std::to_string(desc.miss_shaders.size()));

    for (const auto* miss_shader : desc.miss_shaders)
    {
        uint32_t miss_index = add_shader_stage(miss_shader);

        VkRayTracingShaderGroupCreateInfoKHR miss_group{};
        miss_group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        miss_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        miss_group.generalShader = miss_index;
        miss_group.closestHitShader = VK_SHADER_UNUSED_KHR;
        miss_group.anyHitShader = VK_SHADER_UNUSED_KHR;
        miss_group.intersectionShader = VK_SHADER_UNUSED_KHR;
        groups.push_back(miss_group);

        OGFX_LOG("    Group [" + std::to_string(groups.size() - 1) + "] Type: General");
        OGFX_LOG("      Miss Shader Index: " + std::to_string(miss_index));
    }

    OGFX_LOG("  Hit groups count: " + std::to_string(desc.hit_shaders.size()));

    for (const auto& hit_pair : desc.hit_shaders)
    {
        uint32_t chit_idx = add_shader_stage(hit_pair.closest_hit_shader);
        uint32_t ahit_idx = add_shader_stage(hit_pair.any_hit_shader);
        uint32_t isect_idx = add_shader_stage(hit_pair.intersection_shader);

        VkRayTracingShaderGroupCreateInfoKHR hit_group{};
        hit_group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;

        // Decide hardware intersector path based on presence of custom intersection math
        if (isect_idx != VK_SHADER_UNUSED_KHR)
        {
            hit_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
            OGFX_LOG("    Group [" + std::to_string(groups.size()) + "]: Procedural Hit Group");
        }
        else
        {
            hit_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            OGFX_LOG("    Group [" + std::to_string(groups.size()) + "] Type: Triangles Hit Group");
        }

        if (chit_idx != VK_SHADER_UNUSED_KHR)
        {
            OGFX_LOG("      Closest Hit Shader Index: " + std::to_string(chit_idx));
        }
        if (ahit_idx != VK_SHADER_UNUSED_KHR)
        {
            OGFX_LOG("      Any Hit Shader Index:     " + std::to_string(ahit_idx));
        }
        if (isect_idx != VK_SHADER_UNUSED_KHR)
        {
            OGFX_LOG("      Intersection Shader Index: " + std::to_string(isect_idx));
        }

        hit_group.generalShader = VK_SHADER_UNUSED_KHR;
        hit_group.closestHitShader = chit_idx;
        hit_group.anyHitShader = ahit_idx;
        hit_group.intersectionShader = isect_idx;

        groups.push_back(hit_group);
    }

    VkRayTracingPipelineCreateInfoKHR pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
    pipeline_info.pStages = shader_stages.data();
    pipeline_info.groupCount = static_cast<uint32_t>(groups.size());
    pipeline_info.pGroups = groups.data();
    pipeline_info.maxPipelineRayRecursionDepth = desc.max_recursion_depth;
    pipeline_info.layout = desc.layout->m_impl->m_pipeline_layout;

    VkResult result = vkCreateRayTracingPipelinesKHR(Pipeline::m_impl->m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
                                                     &pipeline_info, nullptr, &Pipeline::m_impl->m_pipeline);
    if (result != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create ray tracing pipeline");
        throw std::runtime_error("Failed to create ray tracing pipeline.");
    }

    // Get the handle size
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_properties{};
    rt_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

    VkPhysicalDeviceProperties2 device_properties{};
    device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    device_properties.pNext = &rt_properties;

    vkGetPhysicalDeviceProperties2(physical_device.m_impl->m_physical_device, &device_properties);

    m_impl->m_raw_handle_size = rt_properties.shaderGroupHandleSize;
    uint32_t base_alignment = rt_properties.shaderGroupBaseAlignment;
    m_impl->m_handle_size = (m_impl->m_raw_handle_size + base_alignment - 1) & ~(base_alignment - 1);

    m_impl->m_miss_count = static_cast<uint32_t>(desc.miss_shaders.size());
    m_impl->m_hit_count = static_cast<uint32_t>(desc.hit_shaders.size());

    OGFX_LOG("Ray tracing pipeline created successfully");
}

RayTracingPipeline::~RayTracingPipeline()
{
    OGFX_LOG("Ray tracing pipeline destroyed");
}

RayTracingPipeline::RayTracingPipeline(RayTracingPipeline&&) noexcept = default;
RayTracingPipeline& RayTracingPipeline::operator=(RayTracingPipeline&&) noexcept = default;

uint32_t RayTracingPipeline::group_count() const
{
    return 1 + m_impl->m_miss_count + m_impl->m_hit_count;
}

uint32_t RayTracingPipeline::sbt_size(uint32_t group_count) const
{
    return m_impl->m_handle_size * group_count;
}

void RayTracingPipeline::write_handles_to_buffer(uint32_t first_group, uint32_t group_count, Buffer& output_buffer) const
{
    uint32_t data_size = group_count * m_impl->m_handle_size;
    std::vector<uint8_t> handles(data_size);
    VkResult result = vkGetRayTracingShaderGroupHandlesKHR(Pipeline::m_impl->m_device, Pipeline::m_impl->m_pipeline,
                                                           first_group, group_count, data_size, handles.data());
    if (result != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to get ray tracing shader group handles");
        return;
    }

    uint32_t sbt_total_size = sbt_size(group_count);

    std::vector<uint8_t> sbt_host_memory(sbt_total_size, 0);
    for (uint32_t i = 0; i < group_count; ++i)
    {
        uint8_t* dst = sbt_host_memory.data() + ((first_group + i) * m_impl->m_handle_size);
        const uint8_t* src = handles.data() + (i * m_impl->m_raw_handle_size);

        std::memcpy(dst, src, m_impl->m_raw_handle_size);
    }

    BufferWriteInfo sbt_write_info;
    sbt_write_info.data = sbt_host_memory.data();
    sbt_write_info.size = sbt_total_size;
    sbt_write_info.offset = static_cast<uint64_t>(first_group) * m_impl->m_handle_size;

    output_buffer.write(sbt_write_info);
}

} // namespace ogfx