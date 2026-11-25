#ifndef OMEGA_PBR_GEOMETRY_HLSLI
#define OMEGA_PBR_GEOMETRY_HLSLI

float geometry_schlick_ggx(float NdotX, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;

    return NdotX / (NdotX * (1.0f - k) + k);
}

float geometry_schlick_ggx_ibl(float NdotX, float roughness)
{
    float a = roughness * roughness;
    float k = a * 0.5f;

    return NdotX / (NdotX * (1.0f - k) + k);
}

float geometry_smith(float NdotV, float NdotL, float roughness)
{
    float ggx_v = geometry_schlick_ggx(NdotV, roughness);
    float ggx_l = geometry_schlick_ggx(NdotL, roughness);

    return ggx_v * ggx_l;
}

float geometry_smith_ibl(float NdotV, float NdotL, float roughness)
{
    float ggx_v = geometry_schlick_ggx_ibl(NdotV, roughness);
    float ggx_l = geometry_schlick_ggx_ibl(NdotL, roughness);
    
    return ggx_v * ggx_l;
}

#endif // OMEGA_PBR_GEOMETRY_HLSLI