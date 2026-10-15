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

    default:
        return ShaderStage::Undefined;
    }
}

} // namespace ogfx