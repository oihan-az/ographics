#ifndef OMEGA_MATERIAL_SLOT_HPP
#define OMEGA_MATERIAL_SLOT_HPP

#include <cstdint>

namespace omega
{

template <typename T> 
struct MaterialSlot
{
    T value{};
    uint32_t generation = 0;
    bool occupied = false;
};

} // namespace omega

#endif // OMEGA_MATERIAL_SLOT_HPP