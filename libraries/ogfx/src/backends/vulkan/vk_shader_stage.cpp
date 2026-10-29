#include "vk_shader_stage.hpp"

namespace ogfx
{

VkShaderStageFlagBits to_vk_shader_stage(ShaderStage stage)
{
    switch (stage)
    {
    case ShaderStage::Vertex:
        return VK_SHADER_STAGE_VERTEX_BIT;

    case ShaderStage::Fragment:
        return VK_SHADER_STAGE_FRAGMENT_BIT;

    case ShaderStage::Compute:
        return VK_SHADER_STAGE_COMPUTE_BIT;

    case ShaderStage::RayGen:
        return VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    case ShaderStage::Miss:
        return VK_SHADER_STAGE_MISS_BIT_KHR;

    case ShaderStage::ClosestHit:
        return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

    case ShaderStage::AnyHit:
        return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;

    case ShaderStage::Intersection:
        return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;

    }

    return static_cast<VkShaderStageFlagBits>(0);
}

ShaderStage from_vk_shader_stage(VkShaderStageFlagBits stage)
{
    switch (stage)
    {
    case VK_SHADER_STAGE_VERTEX_BIT:
        return ShaderStage::Vertex;

    case VK_SHADER_STAGE_FRAGMENT_BIT:
        return ShaderStage::Fragment;

    case VK_SHADER_STAGE_COMPUTE_BIT:
        return ShaderStage::Compute;

    case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        return ShaderStage::RayGen;

    case VK_SHADER_STAGE_MISS_BIT_KHR:
        return ShaderStage::Miss;

    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        return ShaderStage::ClosestHit;

    case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        return ShaderStage::AnyHit;

    case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        return ShaderStage::Intersection;

    default:
        return ShaderStage::Undefined;
    }
}

VkShaderStageFlags to_vk_shader_stage_flags(ShaderStageFlags stages)
{
    VkShaderStageFlags flags = 0;

    if (has_flag(stages, ShaderStageFlags::Vertex))
    {
        flags |= VK_SHADER_STAGE_VERTEX_BIT;
    }

    if (has_flag(stages, ShaderStageFlags::Fragment))
    {
        flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    if (has_flag(stages, ShaderStageFlags::Compute))
    {
        flags |= VK_SHADER_STAGE_COMPUTE_BIT;
    }

    if (has_flag(stages, ShaderStageFlags::RayGen))
    {
        flags |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    }

    if (has_flag(stages, ShaderStageFlags::Miss))
    {
        flags |= VK_SHADER_STAGE_MISS_BIT_KHR;
    }

    if (has_flag(stages, ShaderStageFlags::ClosestHit))
    {
        flags |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    }

    if (has_flag(stages, ShaderStageFlags::AnyHit))
    {
        flags |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    }

    if (has_flag(stages, ShaderStageFlags::Intersection))
    {
        flags |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    }

    return flags;
}

ShaderStageFlags from_vk_shader_stage_flags(VkShaderStageFlags stages)
{
    ShaderStageFlags flags = ShaderStageFlags::None;

    if ((stages & VK_SHADER_STAGE_VERTEX_BIT) != 0)
    {
        flags |= ShaderStageFlags::Vertex;
    }

    if ((stages & VK_SHADER_STAGE_FRAGMENT_BIT) != 0)
    {
        flags |= ShaderStageFlags::Fragment;
    }

    if ((stages & VK_SHADER_STAGE_COMPUTE_BIT) != 0)
    {
        flags |= ShaderStageFlags::Compute;
    }

    if ((stages & VK_SHADER_STAGE_RAYGEN_BIT_KHR) != 0)
    {
        flags |= ShaderStageFlags::RayGen;
    }

    if ((stages & VK_SHADER_STAGE_MISS_BIT_KHR) != 0)
    {
        flags |= ShaderStageFlags::Miss;
    }

    if ((stages & VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) != 0)
    {
        flags |= ShaderStageFlags::ClosestHit;
    }

    if ((stages & VK_SHADER_STAGE_ANY_HIT_BIT_KHR) != 0)
    {
        flags |= ShaderStageFlags::AnyHit;
    }

    if ((stages & VK_SHADER_STAGE_INTERSECTION_BIT_KHR) != 0)
    {
        flags |= ShaderStageFlags::Intersection;
    }

    return flags;
}

} // namespace ogfx