#ifndef OGFX_RASTERIZATION_HPP
#define OGFX_RASTERIZATION_HPP

namespace ogfx
{

enum class PolygonMode
{
    Undefined,
    Fill,
    Line,
    Point
};

enum class CullMode
{
    Undefined,
    None,
    Front,
    Back,
    FrontAndBack
};

enum class FrontFace
{
    Undefined,
    CounterClockwise,
    Clockwise
};

struct RasterizationStateDesc
{
    bool depth_clamp_enable = false;
    bool rasterizer_discard_enable = false;

    PolygonMode polygon_mode = PolygonMode::Fill;
    CullMode cull_mode = CullMode::None;
    FrontFace front_face = FrontFace::CounterClockwise;

    bool depth_bias_enable = false;
    float depth_bias_constant_factor = 0.0f;
    float depth_bias_clamp = 0.0f;
    float depth_bias_slope_factor = 0.0f;

    float line_width = 1.0f;
};

} // namespace ogfx

#endif // OGFX_RASTERIZATION_HPP