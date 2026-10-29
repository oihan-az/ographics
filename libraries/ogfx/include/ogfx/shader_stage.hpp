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
    Compute,
    RayGen,
    Miss,
    ClosestHit,
    AnyHit,
    Intersection
};

OGFX_BITMASK_ENUM(ShaderStageFlags, uint32_t,
    None = 0,
    Vertex = 1 << 0,
    Fragment = 1 << 1,
    Compute = 1 << 2,
    RayGen       = 1 << 3,
    Miss         = 1 << 4,
    ClosestHit   = 1 << 5,
    AnyHit       = 1 << 6,
    Intersection = 1 << 7
);

} // namespace ogfx

#endif // OGFX_SHADER_STAGE_HPP