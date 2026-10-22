#ifndef OGFX_INLINE_CONSTANT_RANGE_HPP
#define OGFX_INLINE_CONSTANT_RANGE_HPP

#include <ogfx/shader_stage.hpp>

#include <cstdint>

namespace ogfx
{

struct InlineConstantRange
{
    ShaderStageFlags stages = ShaderStageFlags::None;
    uint32_t offset = 0;
    uint32_t size = 0;
};

} // namespace ogfx

#endif // OGFX_INLINE_CONSTANT_RANGE_HPP
