#ifndef OMEGA_DITHERING_HLSLI
#define OMEGA_DITHERING_HLSLI

float interleaved_gradient_noise(float2 pixel_position)
{
    return frac(52.9829189f * frac(dot(pixel_position, float2(0.06711056f, 0.00583715f))));
}

float3 dither_8bit(float3 color, float2 pixel_position)
{
    float noise = interleaved_gradient_noise(pixel_position) - 0.5f;
    return saturate(color + noise / 255.0f);
}

#endif // OMEGA_DITHERING_HLSLI
