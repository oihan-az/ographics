#ifndef OGFX_COLOR_BLEND_HPP
#define OGFX_COLOR_BLEND_HPP

#include <ogfx/enum_flags.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace ogfx
{

enum class BlendFactor
{
    Undefined,
    Zero,
    One,
    SourceColor,
    OneMinusSourceColor,
    DestinationColor,
    OneMinusDestinationColor,
    SourceAlpha,
    OneMinusSourceAlpha,
    DestinationAlpha,
    OneMinusDestinationAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
    SourceAlphaSaturate,
    Source1Color,
    OneMinusSource1Color,
    Source1Alpha,
    OneMinusSource1Alpha
};

enum class BlendOp
{
    Undefined,
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max
};

OGFX_BITMASK_ENUM(ColorComponent, uint32_t,
    None = 0,
    Red = 1u << 0,
    Green = 1u << 1,
    Blue = 1u << 2,
    Alpha = 1u << 3,
    All = Red | Green | Blue | Alpha
);

struct ColorBlendAttachmentDesc
{
    bool blend_enable = false;

    BlendFactor source_color_factor = BlendFactor::One;
    BlendFactor destination_color_factor = BlendFactor::Zero;
    BlendOp color_op = BlendOp::Add;

    BlendFactor source_alpha_factor = BlendFactor::One;
    BlendFactor destination_alpha_factor = BlendFactor::Zero;
    BlendOp alpha_op = BlendOp::Add;

    ColorComponent write_mask = ColorComponent::All;
};

struct ColorBlendStateDesc
{
    bool logic_op_enable = false;
    std::vector<ColorBlendAttachmentDesc> attachments;

    float blend_constants[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

} // namespace ogfx

#endif // OGFX_COLOR_BLEND_HPP