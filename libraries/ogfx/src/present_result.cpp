#include <ogfx/present_result.hpp>

namespace ogfx
{

const char* to_string(PresentResult result)
{
    switch (result)
    {
    case PresentResult::Success:
        return "Success";

    case PresentResult::Suboptimal:
        return "Suboptimal";

    case PresentResult::OutOfDate:
        return "OutOfDate";

    default:
        return "Unknown";
    }
}

} // namespace ogfx