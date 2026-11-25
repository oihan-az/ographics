#ifndef OMEGA_COLOR_SRGB_HLSLI
#define OMEGA_COLOR_SRGB_HLSLI

float3 linear_to_srgb(float3 color)
{
    color = saturate(color);

    float3 low = color * 12.92f;
    float3 high = 1.055f * pow(color, 1.0f / 2.4f) - 0.055f;

    return float3(
        color.r <= 0.0031308f ? low.r : high.r,
        color.g <= 0.0031308f ? low.g : high.g,
        color.b <= 0.0031308f ? low.b : high.b
    );
}

float3 srgb_to_linear(float3 color)
{
    color = saturate(color);

    float3 low = color / 12.92f;
    float3 high = pow((color + 0.055f) / 1.055f, 2.4f);

    return float3(
        color.r <= 0.04045f ? low.r : high.r,
        color.g <= 0.04045f ? low.g : high.g,
        color.b <= 0.04045f ? low.b : high.b
    );
}

#endif // OMEGA_COLOR_SRGB_HLSLI