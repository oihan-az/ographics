#ifndef OGFX_COMPARE_OP_HPP
#define OGFX_COMPARE_OP_HPP

namespace ogfx
{

enum class CompareOp
{
    Undefined,
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always
};

} // namespace ogfx

#endif // OGFX_COMPARE_OP_HPP