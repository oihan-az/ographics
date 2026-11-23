#ifndef OMEGA_PBR_BRDF_HLSLI
#define OMEGA_PBR_BRDF_HLSLI

#include "pbr_surface.hlsli"
#include "fresnel.hlsli"
#include "distribution.hlsli"
#include "geometry.hlsli"

float3 evaluate_pbr_brdf(PbrSurface surface, float3 light_direction, float3 radiance)
{
    float3 N = normalize(surface.normal);
    float3 V = normalize(surface.view_direction);
    float3 L = normalize(light_direction);

    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));
    if (NdotL <= 0.0f || NdotV <= 0.0f)
        return float3(0.0f, 0.0f, 0.0f);
    
    float3 H = normalize(V + L);

    float metallic = saturate(surface.metallic);
    float roughness = clamp(surface.roughness, 0.04f, 1.0f);

    // Base reflectance
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), surface.base_color, metallic);

    // Cook-Torrance specular BRDF
    float NDF = distribution_ggx(N, H, roughness);
    float G = geometry_smith(N, V, L, roughness);
    float3 F = fresnel_schlick(saturate(dot(H, V)), F0);

    float3 numerator = NDF * G * F;
    float denominator = max(4.0f * NdotV * NdotL, 0.0001f);

    float3 specular = numerator / denominator;

    // Energy-conserving diffuse contribution
    float3 kS = F;
    float3 kD = (1.0f - kS) * (1.0f - metallic);

    float3 diffuse = kD * surface.base_color / PBR_PI;

    // Outgoing reflected radiance
    return (diffuse + specular) * radiance * NdotL;
}

#endif // OMEGA_PBR_BRDF_HLSLI