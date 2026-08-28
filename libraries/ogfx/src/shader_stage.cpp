#include <ogfx/shader_stage.hpp>

namespace ogfx
{

const char* to_string(ShaderStage stage)
{
    switch (stage)
    {
    case ShaderStage::Vertex:
        return "Vertex";

    case ShaderStage::Fragment:
        return "Fragment";

    case ShaderStage::Compute:
        return "Compute";

    default:
        return "Unknown";
    }
}

} // namespace ogfx