#include <ogfx/present_mode.hpp>

namespace ogfx
{

const char* to_string(PresentMode mode)
{
    switch (mode)
    {
    case PresentMode::Immediate:
        return "Immediate";

    case PresentMode::FIFO:
        return "FIFO";

    case PresentMode::Mailbox:
        return "Mailbox";
    }

    return "Unknown";
}

} // namespace ogfx