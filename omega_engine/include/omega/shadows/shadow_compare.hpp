#ifndef OMEGA_SHADOW_COMPARE_HPP
#define OMEGA_SHADOW_COMPARE_HPP

namespace omega
{

struct ShadowCompare
{
    float depth_bias = 0.001f;
    float normal_bias = 0.01f;
};

} // namespace omega

#endif // OMEGA_SHADOW_COMPARE_HPP