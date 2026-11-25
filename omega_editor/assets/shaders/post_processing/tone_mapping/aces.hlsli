#ifndef OMEGA_COLOR_ACES_HLSLI
#define OMEGA_COLOR_ACES_HLSLI

// Stephen Hill's fitted approximation of ACES RRT + ODT.
// Input: linear HDR RGB with sRGB/Rec.709 primaries.
// Output: linear SDR RGB with sRGB/Rec.709 primaries.

static const float3x3 ACESInputMat =
{
    { 0.59719f, 0.35458f, 0.04823f },
    { 0.07600f, 0.90834f, 0.01566f },
    { 0.02840f, 0.13383f, 0.83777f }
};

static const float3x3 ACESOutputMat =
{
    { 1.60475f, -0.53108f, -0.07367f },
    { -0.10208f, 1.10813f, -0.00605f },
    { -0.00327f, -0.07276f, 1.07602f }
};

float3 aces_rrt_odt_fit(float3 color)
{
    float3 a = color * (color + 0.0245786f) - 0.000090537f;
    float3 b = color * (0.983729f * color + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 tonemap_aces(float3 color)
{
    color = max(color, 0.0f);

    // Linear sRGB to ACES-derived working space
    color = mul(ACESInputMat, color);

    // Fitted ACES rendering and display transform
    color = aces_rrt_odt_fit(color);

    // Working space to linear sRGB
    color = mul(ACESOutputMat, color);

    return saturate(color);
}

#endif // OMEGA_COLOR_ACES_HLSLI