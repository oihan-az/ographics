#ifndef OGFX_SHADER_STAGE_HPP
#define OGFX_SHADER_STAGE_HPP

#include <ogfx/enum_flags.hpp>

namespace ogfx
{

enum class ShaderStage
{
    Undefined,
    Vertex,
    Fragment,
    Compute
};

OGFX_BITMASK_ENUM(ShaderStageFlags, uint32_t,
    None = 0,
    Vertex = 1 << 0,
    Fragment = 1 << 1,
    Compute = 1 << 2
);

} // namespace ogfx

#endif // OGFX_SHADER_STAGE_HPP