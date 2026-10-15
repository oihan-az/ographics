#include "vk_color_blend.hpp"

namespace ogfx
{

VkBlendFactor to_vk_blend_factor(BlendFactor factor)
{
    switch (factor)
    {
    case BlendFactor::Zero:
        return VK_BLEND_FACTOR_ZERO;
    case BlendFactor::One:
        return VK_BLEND_FACTOR_ONE;
    case BlendFactor::SourceColor:
        return VK_BLEND_FACTOR_SRC_COLOR;
    case BlendFactor::OneMinusSourceColor:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case BlendFactor::DestinationColor:
        return VK_BLEND_FACTOR_DST_COLOR;
    case BlendFactor::OneMinusDestinationColor:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case BlendFactor::SourceAlpha:
        return VK_BLEND_FACTOR_SRC_ALPHA;
    case BlendFactor::OneMinusSourceAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::DestinationAlpha:
        return VK_BLEND_FACTOR_DST_ALPHA;
    case BlendFactor::OneMinusDestinationAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case BlendFactor::ConstantColor:
        return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case BlendFactor::OneMinusConstantColor:
        return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case BlendFactor::ConstantAlpha:
        return VK_BLEND_FACTOR_CONSTANT_ALPHA;
    case BlendFactor::OneMinusConstantAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    case BlendFactor::SourceAlphaSaturate:
        return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case BlendFactor::Source1Color:
        return VK_BLEND_FACTOR_SRC1_COLOR;
    case BlendFactor::OneMinusSource1Color:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case BlendFactor::Source1Alpha:
        return VK_BLEND_FACTOR_SRC1_ALPHA;
    case BlendFactor::OneMinusSource1Alpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    case BlendFactor::Undefined:
        return VK_BLEND_FACTOR_MAX_ENUM;
    }

    return VK_BLEND_FACTOR_MAX_ENUM;
}

BlendFactor from_vk_blend_factor(VkBlendFactor factor)
{
    switch (factor)
    {
    case VK_BLEND_FACTOR_ZERO:
        return BlendFactor::Zero;
    case VK_BLEND_FACTOR_ONE:
        return BlendFactor::One;
    case VK_BLEND_FACTOR_SRC_COLOR:
        return BlendFactor::SourceColor;
    case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
        return BlendFactor::OneMinusSourceColor;
    case VK_BLEND_FACTOR_DST_COLOR:
        return BlendFactor::DestinationColor;
    case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
        return BlendFactor::OneMinusDestinationColor;
    case VK_BLEND_FACTOR_SRC_ALPHA:
        return BlendFactor::SourceAlpha;
    case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        return BlendFactor::OneMinusSourceAlpha;
    case VK_BLEND_FACTOR_DST_ALPHA:
        return BlendFactor::DestinationAlpha;
    case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
        return BlendFactor::OneMinusDestinationAlpha;
    case VK_BLEND_FACTOR_CONSTANT_COLOR:
        return BlendFactor::ConstantColor;
    case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
        return BlendFactor::OneMinusConstantColor;
    case VK_BLEND_FACTOR_CONSTANT_ALPHA:
        return BlendFactor::ConstantAlpha;
    case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
        return BlendFactor::OneMinusConstantAlpha;
    case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
        return BlendFactor::SourceAlphaSaturate;
    case VK_BLEND_FACTOR_SRC1_COLOR:
        return BlendFactor::Source1Color;
    case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
        return BlendFactor::OneMinusSource1Color;
    case VK_BLEND_FACTOR_SRC1_ALPHA:
        return BlendFactor::Source1Alpha;
    case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
        return BlendFactor::OneMinusSource1Alpha;
    default:
        return BlendFactor::Undefined;
    }
}

VkBlendOp to_vk_blend_op(BlendOp op)
{
    switch (op)
    {
    case BlendOp::Add:
        return VK_BLEND_OP_ADD;
    case BlendOp::Subtract:
        return VK_BLEND_OP_SUBTRACT;
    case BlendOp::ReverseSubtract:
        return VK_BLEND_OP_REVERSE_SUBTRACT;
    case BlendOp::Min:
        return VK_BLEND_OP_MIN;
    case BlendOp::Max:
        return VK_BLEND_OP_MAX;
    case BlendOp::Undefined:
        return VK_BLEND_OP_MAX_ENUM;
    }

    return VK_BLEND_OP_MAX_ENUM;
}

BlendOp from_vk_blend_op(VkBlendOp op)
{
    switch (op)
    {
    case VK_BLEND_OP_ADD:
        return BlendOp::Add;
    case VK_BLEND_OP_SUBTRACT:
        return BlendOp::Subtract;
    case VK_BLEND_OP_REVERSE_SUBTRACT:
        return BlendOp::ReverseSubtract;
    case VK_BLEND_OP_MIN:
        return BlendOp::Min;
    case VK_BLEND_OP_MAX:
        return BlendOp::Max;
    default:
        return BlendOp::Undefined;
    }
}

VkColorComponentFlags to_vk_color_components(ColorComponent components)
{
    VkColorComponentFlags flags = 0;

    if (has_flag(components, ColorComponent::Red))
        flags |= VK_COLOR_COMPONENT_R_BIT;

    if (has_flag(components, ColorComponent::Green))
        flags |= VK_COLOR_COMPONENT_G_BIT;

    if (has_flag(components, ColorComponent::Blue))
        flags |= VK_COLOR_COMPONENT_B_BIT;

    if (has_flag(components, ColorComponent::Alpha))
        flags |= VK_COLOR_COMPONENT_A_BIT;

    return flags;
}

ColorComponent from_vk_color_components(VkColorComponentFlags components)
{
    ColorComponent result = ColorComponent::None;

    if (components & VK_COLOR_COMPONENT_R_BIT)
        result = result | ColorComponent::Red;

    if (components & VK_COLOR_COMPONENT_G_BIT)
        result = result | ColorComponent::Green;

    if (components & VK_COLOR_COMPONENT_B_BIT)
        result = result | ColorComponent::Blue;

    if (components & VK_COLOR_COMPONENT_A_BIT)
        result = result | ColorComponent::Alpha;

    return result;
}

} // namespace ogfx