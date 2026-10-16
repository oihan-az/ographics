#ifndef OGFX_PRESENT_RESULT_HPP
#define OGFX_PRESENT_RESULT_HPP

namespace ogfx
{

enum class PresentResult
{
    Success,
    Suboptimal,
    OutOfDate
};

} // namespace ogfx

#endif // OGFX_PRESENT_RESULT_HPP