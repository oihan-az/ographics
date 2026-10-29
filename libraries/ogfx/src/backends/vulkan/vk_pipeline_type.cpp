#include "vk_pipeline_type.hpp"

namespace ogfx
{

VkPipelineBindPoint to_vk_pipeline_type(PipelineType pipeline_type)
{
    switch (pipeline_type)
    {
    case PipelineType::Graphics:
        return VK_PIPELINE_BIND_POINT_GRAPHICS;

    case PipelineType::Compute:
        return VK_PIPELINE_BIND_POINT_COMPUTE;

    case PipelineType::RayTracing:
        return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;

    default:
        return VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }
}

PipelineType from_vk_pipeline_type(VkPipelineBindPoint pipeline_type)
{
    switch (pipeline_type)
    {
    case VK_PIPELINE_BIND_POINT_GRAPHICS:
        return PipelineType::Graphics;

    case VK_PIPELINE_BIND_POINT_COMPUTE:
        return PipelineType::Compute;

    case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
        return PipelineType::RayTracing;

    default:
        return PipelineType::Undefined;
    }
}

} // namespace ogfx