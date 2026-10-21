#ifndef OGFX_VK_SHADER_STAGE_HPP
#define OGFX_VK_SHADER_STAGE_HPP

#include <ogfx/shader_stage.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkShaderStageFlagBits to_vk_shader_stage(ShaderStage stage);
ShaderStage from_vk_shader_stage(VkShaderStageFlagBits stage);


VkShaderStageFlags to_vk_shader_stage_flags(ShaderStageFlags stages);
ShaderStageFlags from_vk_shader_stage_flags(VkShaderStageFlags stages);

} // namespace ogfx

#endif // OGFX_VK_SHADER_STAGE_HPP