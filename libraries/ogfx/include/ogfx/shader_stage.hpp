#ifndef OGFX_SHADER_STAGE_HPP
#define OGFX_SHADER_STAGE_HPP

namespace ogfx
{

enum class ShaderStage
{
    Vertex,
    Fragment,
    Compute
};

const char* to_string(ShaderStage stage);

} // namespace ogfx

#endif // OGFX_SHADER_STAGE_HPP