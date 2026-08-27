#include <ogfx/image_state.hpp>

namespace ogfx
{

const char* to_string(ImageState state)
{
    switch (state)
    {
    case ImageState::Undefined:
        return "Undefined";

    case ImageState::Common:
        return "Common";

    case ImageState::RenderTarget:
        return "RenderTarget";

    case ImageState::DepthStencil:
        return "DepthStencil";

    case ImageState::ShaderRead:
        return "ShaderRead";

    case ImageState::TransferSrc:
        return "TransferSrc";

    case ImageState::TransferDst:
        return "TransferDst";

    case ImageState::Present:
        return "Present";

    default:
        return "Unknown";
    }
}

} // namespace ogfx