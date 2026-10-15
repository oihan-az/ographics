#include <ogfx/color_blend.hpp>

namespace ogfx
{

const char* to_string(BlendFactor factor)
{
    switch (factor)
    {
    case BlendFactor::Undefined:
        return "Undefined";
    case BlendFactor::Zero:
        return "Zero";
    case BlendFactor::One:
        return "One";
    case BlendFactor::SourceColor:
        return "SourceColor";
    case BlendFactor::OneMinusSourceColor:
        return "OneMinusSourceColor";
    case BlendFactor::DestinationColor:
        return "DestinationColor";
    case BlendFactor::OneMinusDestinationColor:
        return "OneMinusDestinationColor";
    case BlendFactor::SourceAlpha:
        return "SourceAlpha";
    case BlendFactor::OneMinusSourceAlpha:
        return "OneMinusSourceAlpha";
    case BlendFactor::DestinationAlpha:
        return "DestinationAlpha";
    case BlendFactor::OneMinusDestinationAlpha:
        return "OneMinusDestinationAlpha";
    case BlendFactor::ConstantColor:
        return "ConstantColor";
    case BlendFactor::OneMinusConstantColor:
        return "OneMinusConstantColor";
    case BlendFactor::ConstantAlpha:
        return "ConstantAlpha";
    case BlendFactor::OneMinusConstantAlpha:
        return "OneMinusConstantAlpha";
    case BlendFactor::SourceAlphaSaturate:
        return "SourceAlphaSaturate";
    case BlendFactor::Source1Color:
        return "Source1Color";
    case BlendFactor::OneMinusSource1Color:
        return "OneMinusSource1Color";
    case BlendFactor::Source1Alpha:
        return "Source1Alpha";
    case BlendFactor::OneMinusSource1Alpha:
        return "OneMinusSource1Alpha";
    }

    return "Unknown";
}

const char* to_string(BlendOp op)
{
    switch (op)
    {
    case BlendOp::Undefined:
        return "Undefined";
    case BlendOp::Add:
        return "Add";
    case BlendOp::Subtract:
        return "Subtract";
    case BlendOp::ReverseSubtract:
        return "ReverseSubtract";
    case BlendOp::Min:
        return "Min";
    case BlendOp::Max:
        return "Max";
    }

    return "Unknown";
}

std::string to_string(ColorComponent component)
{
    if (component == ColorComponent::None)
        return "None";

    if (component == ColorComponent::All)
        return "All";

    std::string result;

    if (has_flag(component, ColorComponent::Red))
        result += "Red | ";

    if (has_flag(component, ColorComponent::Green))
        result += "Green | ";

    if (has_flag(component, ColorComponent::Blue))
        result += "Blue | ";

    if (has_flag(component, ColorComponent::Alpha))
        result += "Alpha | ";

    if (result.empty())
        return "Unknown";

    result.resize(result.size() - 3);

    return result;
}

} // namespace ogfx