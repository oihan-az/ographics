#include <ogfx/memory_property.hpp>

namespace ogfx
{

std::string to_string(MemoryProperty property)
{
    if (property == MemoryProperty::None)
    {
        return "None";
    }

    std::string result;

    if (has_flag(property, MemoryProperty::DeviceLocal))
    {
        result += "DeviceLocal | ";
    }

    if (has_flag(property, MemoryProperty::HostVisible))
    {
        result += "HostVisible | ";
    }

    if (has_flag(property, MemoryProperty::HostCoherent))
    {
        result += "HostCoherent | ";
    }

    if (has_flag(property, MemoryProperty::HostCached))
    {
        result += "HostCached | ";
    }

    if (result.empty())
    {
        return "Unknown";
    }

    result.resize(result.size() - 3);
    return result;
}

} // namespace ogfx