#include <ogfx/rasterization.hpp>

namespace ogfx
{

const char* to_string(PolygonMode mode)
{
    switch (mode)
    {
    case PolygonMode::Undefined:
        return "Undefined";
    case PolygonMode::Fill:
        return "Fill";
    case PolygonMode::Line:
        return "Line";
    case PolygonMode::Point:
        return "Point";
    }

    return "Unknown";
}

const char* to_string(CullMode mode)
{
    switch (mode)
    {
    case CullMode::Undefined:
        return "Undefined";
    case CullMode::None:
        return "None";
    case CullMode::Front:
        return "Front";
    case CullMode::Back:
        return "Back";
    case CullMode::FrontAndBack:
        return "FrontAndBack";
    }

    return "Unknown";
}

const char* to_string(FrontFace face)
{
    switch (face)
    {
    case FrontFace::Undefined:
        return "Undefined";
    case FrontFace::CounterClockwise:
        return "CounterClockwise";
    case FrontFace::Clockwise:
        return "Clockwise";
    }

    return "Unknown";
}

} // namespace ogfx