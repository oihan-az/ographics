#include <ogfx/load_store_op.hpp>

namespace ogfx
{

const char* to_string(LoadOp op)
{
    switch (op)
    {
    case LoadOp::Load:
        return "Load";

    case LoadOp::Clear:
        return "Clear";

    case LoadOp::DontCare:
        return "DontCare";

    default:
        return "Unknown";
    }
}

const char* to_string(StoreOp op)
{
    switch (op)
    {
    case StoreOp::Store:
        return "Store";

    case StoreOp::DontCare:
        return "DontCare";

    default:
        return "Unknown";
    }
}

} // namespace ogfx