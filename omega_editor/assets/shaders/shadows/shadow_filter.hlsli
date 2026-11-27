#ifndef OMEGA_SHADOW_FILTER_HLSLI
#define OMEGA_SHADOW_FILTER_HLSLI

#include "../hashing/hash.hlsli"

static const uint SHADOW_FILTER_HARD = 0;
static const uint SHADOW_FILTER_PCF = 1;
static const uint SHADOW_FILTER_PCSS = 2;

static const uint SHADOW_MAX_SAMPLES = 64;

static const float SHADOW_TWO_PI = 6.28318530718f;

struct ShadowFilter
{
    // x: PCF radius in texels
    // y: light angular radius (directional) or world radius (local)
    // z: maximum PCSS radius in texels
    // w: unused
    float4 parameters;

    // x: filter mode
    // y: blocker samples
    // z: filter samples
    // w: unused
    uint4 options;
};

uint shadow_sample_count(uint count)
{
    return clamp(count, 1u, SHADOW_MAX_SAMPLES);
}

float2 shadow_disk_offset(uint index, uint count)
{
    const float golden_angle = 2.39996323f;
    const float radius = sqrt((float(index) + 0.5f) / float(max(count, 1u)));
    const float angle = float(index) * golden_angle;

    return radius * float2(cos(angle), sin(angle));
}

float2 shadow_rotation(float2 screen_position)
{
    const float angle = hash_screen_position(screen_position) * SHADOW_TWO_PI;
    return float2(cos(angle), sin(angle));
}

float2 rotate_shadow_offset(float2 offset, float2 rotation)
{
    return float2(
        offset.x * rotation.x - offset.y * rotation.y,
        offset.x * rotation.y + offset.y * rotation.x
    );
}

float2 shadow_disk_offset(uint index, uint count, float2 rotation)
{
    return rotate_shadow_offset(shadow_disk_offset(index, count), rotation);
}

#endif // OMEGA_SHADOW_FILTER_HLSLI