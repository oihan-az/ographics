#ifndef OGFX_IMAGE_STATE_HPP
#define OGFX_IMAGE_STATE_HPP

namespace ogfx
{

enum class ImageState
{
    Undefined,
    Common,
    RenderTarget,
    DepthStencil,
    ShaderRead,
    TransferSrc,
    TransferDst,
    Present
};

const char* to_string(ImageState state);

} // namespace ogfx

#endif // OGFX_IMAGE_STATE_HPP