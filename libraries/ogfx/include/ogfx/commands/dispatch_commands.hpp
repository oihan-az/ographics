#ifndef OGFX_DISPATCH_COMMANDS_HPP
#define OGFX_DISPATCH_COMMANDS_HPP

#include <cstdint>

namespace ogfx
{

struct DispatchInfo
{
    uint32_t group_count_x = 1;
    uint32_t group_count_y = 1;
    uint32_t group_count_z = 1;
};

} // namespace ogfx

#endif // OGFX_DISPATCH_COMMANDS_HPP