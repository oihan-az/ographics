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

const char* to_string(PresentResult result);

} // namespace ogfx

#endif // OGFX_PRESENT_RESULT_HPP