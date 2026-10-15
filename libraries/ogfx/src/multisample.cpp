#include <ogfx/multisample.hpp>

namespace ogfx
{

const char* to_string(SampleCount count)
{
    switch (count)
    {
    case SampleCount::Undefined:
        return "Undefined";
    case SampleCount::Count1:
        return "1";
    case SampleCount::Count2:
        return "2";
    case SampleCount::Count4:
        return "4";
    case SampleCount::Count8:
        return "8";
    case SampleCount::Count16:
        return "16";
    case SampleCount::Count32:
        return "32";
    case SampleCount::Count64:
        return "64";
    }

    return "Unknown";
}

} // namespace ogfx