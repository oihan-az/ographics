#ifndef OMEGA_PBR_FRESNEL_HLSLI
#define OMEGA_PBR_FRESNEL_HLSLI

float3 base_reflectance(float3 base_color, float metallic)
{
    return lerp(float3(0.04f, 0.04f, 0.04f), base_color, metallic);
}

float3 fresnel_schlick(float cos_theta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - saturate(cos_theta), 5.0f);
}

float3 fresnel_roughness(float cos_theta, float3 F0, float roughness)
{
    float3 F90 = max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0);
    return F0 + (F90 - F0) * pow(1.0f - cos_theta, 5.0f);
}

#endif // OMEGA_PBR_FRESNEL_HLSLI