#ifndef OMEGA_PBR_DISTRIBUTION_HLSLI
#define OMEGA_PBR_DISTRIBUTION_HLSLI

static const float PBR_PI = 3.14159265359f;

float distribution_ggx(float3 N, float3 H, float roughness)
{
    roughness = clamp(roughness, 0.04f, 1.0f);
    
    float a = roughness * roughness;
    float a2 = a * a;

    float NdotH = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;

    float denominator = NdotH2 * (a2 - 1.0f) + 1.0f;

    return a2 / (PBR_PI * denominator * denominator);
}

#endif // OMEGA_PBR_DISTRIBUTION_HLSLI