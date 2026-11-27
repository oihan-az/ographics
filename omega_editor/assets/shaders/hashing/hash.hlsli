#ifndef OMEGA_HASH_HLSLI
#define OMEGA_HASH_HLSLI

uint hash_uint(uint value)
{
    value ^= value >> 16;
    value *= 0x7feb352du;
    value ^= value >> 15;
    value *= 0x846ca68bu;
    value ^= value >> 16;

    return value;
}

uint hash_uint2(uint2 value)
{
    return hash_uint(value.x ^ (hash_uint(value.y) + 0x9e3779b9u));
}

float hash_float01(uint value)
{
    return float(hash_uint(value) & 0x00ffffffu) * (1.0f / 16777216.0f);
}

float hash_screen_position(float2 screen_position)
{
    const uint2 pixel = uint2(screen_position);
    return hash_float01(hash_uint2(pixel));
}

#endif // OMEGA_HASH_HLSLI