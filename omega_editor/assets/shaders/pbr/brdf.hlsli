#ifndef OMEGA_PBR_BRDF_HLSLI
#define OMEGA_PBR_BRDF_HLSLI

#include "pbr_surface.hlsli"
#include "fresnel.hlsli"
#include "distribution.hlsli"
#include "geometry.hlsli"
#include "energy.hlsli"

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
    float NdotH = saturate(dot(N, H));
    float VdotH = saturate(dot(V, H));

    // Base reflectance
    float3 F0 = base_reflectance(surface.base_color, surface.metallic);

    // Cook-Torrance specular BRDF
    float NDF = distribution_ggx(NdotH, surface.roughness);
    float G = geometry_smith(NdotV, NdotL, surface.roughness);
    float3 F = fresnel_schlick(VdotH, F0);

    float3 numerator = NDF * G * F;
    float denominator = max(4.0f * NdotV * NdotL, 0.0001f);

    float3 specular = numerator / denominator;

    float3 kD = energy_conservation(F, surface.metallic);
    float3 diffuse = kD * surface.base_color / PBR_PI;

    // Outgoing reflected radiance
    return (diffuse + specular) * radiance * NdotL;
}

#endif // OMEGA_PBR_BRDF_HLSLI