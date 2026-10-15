#ifndef OGFX_COLOR_BLEND_HPP
#define OGFX_COLOR_BLEND_HPP

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

const char* to_string(BlendFactor factor);

enum class BlendOp
{
    Undefined,
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max
};

const char* to_string(BlendOp op);

enum class ColorComponent : uint32_t
{
    None = 0,
    Red = 1 << 0,
    Green = 1 << 1,
    Blue = 1 << 2,
    Alpha = 1 << 3,
    All = Red | Green | Blue | Alpha
};

constexpr ColorComponent operator|(ColorComponent lhs, ColorComponent rhs)
{
    return static_cast<ColorComponent>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr bool has_flag(ColorComponent value, ColorComponent flag)
{
    return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) == static_cast<uint32_t>(flag);
}

std::string to_string(ColorComponent component);

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