#ifndef OGFX_MULTISAMPLE_HPP
#define OGFX_MULTISAMPLE_HPP

#include <cstdint>

namespace ogfx
{

enum class SampleCount : uint32_t
{
    Undefined = 0,
    Count1 = 1,
    Count2 = 2,
    Count4 = 4,
    Count8 = 8,
    Count16 = 16,
    Count32 = 32,
    Count64 = 64
};

struct MultisampleStateDesc
{
    SampleCount rasterization_samples = SampleCount::Count1;
    bool sample_shading_enable = false;
    float min_sample_shading = 1.0f;
    bool alpha_to_coverage_enable = false;
    bool alpha_to_one_enable = false;
};

} // namespace ogfx

#endif // OGFX_MULTISAMPLE_HPP