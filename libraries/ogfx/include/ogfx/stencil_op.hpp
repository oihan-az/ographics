#ifndef OGFX_STENCIL_OP_HPP
#define OGFX_STENCIL_OP_HPP

namespace ogfx
{

enum class StencilOp
{
    Undefined,
    Keep,
    Zero,
    Replace,
    IncrementAndClamp,
    DecrementAndClamp,
    Invert,
    IncrementAndWrap,
    DecrementAndWrap
};

} // namespace ogfx

#endif // OGFX_STENCIL_OP_HPP