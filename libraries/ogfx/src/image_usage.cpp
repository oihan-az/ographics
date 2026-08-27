#include <ogfx/image_usage.hpp>

namespace ogfx
{

std::string to_string(ImageUsage usage)
{
    if (usage == ImageUsage::None)
    {
        return "None";
    }

    std::string result;

    auto append = [&](ImageUsage flag, const char* name)
    {
        if (!has_flag(usage, flag))
        {
            return;
        }

        if (!result.empty())
        {
            result += " | ";
        }

        result += name;
    };

    append(ImageUsage::TransferSrc, "TransferSrc");
    append(ImageUsage::TransferDst, "TransferDst");
    append(ImageUsage::Sampled, "Sampled");
    append(ImageUsage::Storage, "Storage");
    append(ImageUsage::ColorAttachment, "ColorAttachment");
    append(ImageUsage::DepthStencil, "DepthStencil");

    return result;
}

} // namespace ogfx