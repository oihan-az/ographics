#ifndef OMEGA_SHADOW_LOOKUP_HLSLI
#define OMEGA_SHADOW_LOOKUP_HLSLI

static const uint INVALID_SHADOW_INDEX = 0xffffffffu;

struct ShadowLookup
{
    uint data_index;
    uint texture_index;
};

bool has_shadow(ShadowLookup lookup)
{
    return lookup.data_index != INVALID_SHADOW_INDEX &&
           lookup.texture_index != INVALID_SHADOW_INDEX;
}

#endif // OMEGA_SHADOW_LOOKUP_HLSLI