#ifndef OGFX_INLINE_CONSTANTS_COMMANDS_HPP
#define OGFX_INLINE_CONSTANTS_COMMANDS_HPP

#include <ogfx/shader_stage.hpp>

#include <cstdint>

namespace ogfx
{

class PipelineLayout;

struct InlineConstantsInfo
{
    const PipelineLayout* layout = nullptr;
    ShaderStageFlags stages = ShaderStageFlags::None;

    uint32_t offset = 0;
    uint32_t size = 0;
    const void* data = nullptr;
};

} // namespace ogfx

#endif // OGFX_INLINE_CONSTANTS_COMMANDS_HPP