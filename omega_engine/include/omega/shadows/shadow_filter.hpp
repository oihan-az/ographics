#ifndef OMEGA_SHADOW_FILTER_HPP
#define OMEGA_SHADOW_FILTER_HPP

#include <cstdint>

namespace omega
{

enum class ShadowFilterMode : uint32_t
{
    Hard = 0,
    PCF = 1,
    PCSS = 2
};

struct ShadowFilter
{
    ShadowFilterMode mode = ShadowFilterMode::PCF;

    // PCF kernel radius, measured in shadow-map texels
    float radius = 1.5f;

    // Approximate emitter size for PCSS
    // Directional: angular radius in radians
    // Spot/point: emitter radius in world units
    float light_size = 0.05f;

    // Maximum PCSS filter radius, in shadow-map texels
    float max_radius = 16.0f;

    // PCSS blocker search samples
    uint32_t blocker_samples = 16;

    // PCF filtering samples, also used by PCSS
    uint32_t filter_samples = 16;
};

} // namespace omega

#endif // OMEGA_SHADOW_FILTER_HPP