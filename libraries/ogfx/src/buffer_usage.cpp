#include <ogfx/buffer_usage.hpp>

namespace ogfx
{

std::string to_string(BufferUsage usage)
{
    if (usage == BufferUsage::None)
        return "None";

    std::string result;

    if (has_flag(usage, BufferUsage::Vertex))
        result += "Vertex | ";

    if (has_flag(usage, BufferUsage::Index))
        result += "Index | ";

    if (has_flag(usage, BufferUsage::Uniform))
        result += "Uniform | ";

    if (has_flag(usage, BufferUsage::Storage))
        result += "Storage | ";

    if (has_flag(usage, BufferUsage::TransferSrc))
        result += "TransferSrc | ";

    if (has_flag(usage, BufferUsage::TransferDst))
        result += "TransferDst | ";

    if (result.empty())
        return "Unknown";

    result.resize(result.size() - 3);

    return result;
}

} // namespace ogfx