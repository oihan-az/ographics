#ifndef OMEGA_PBR_IBL_HLSLI
#define OMEGA_PBR_IBL_HLSLI

#include "pbr_surface.hlsli"
#include "fresnel.hlsli"
#include "energy.hlsli"

float2 environment_brdf(float NdotV, float roughness, Texture2D<half2> brdf_lut, SamplerState brdf_lut_sampler)
{
    return brdf_lut.SampleLevel(brdf_lut_sampler, float2(NdotV, roughness), 0.0f).rg;
}

float3 evaluate_specular_ibl(
    PbrSurface surface, 
    TextureCube<half4> environment_map, 
    SamplerState environment_sampler,
    Texture2D<half2> brdf_lut, 
    SamplerState brdf_lut_sampler)
{
    float3 N = normalize(surface.normal);
    float3 V = normalize(surface.view_direction);
    float3 R = reflect(-V, N);
    
    float NdotV = saturate(dot(N, V));
    
    uint width, height, mip_count;
    environment_map.GetDimensions(0, width, height, mip_count);
    float lod = surface.roughness * float(mip_count - 1);

    float3 radiance = environment_map.SampleLevel(environment_sampler, R, lod).rgb;
    float2 brdf = environment_brdf(NdotV, surface.roughness, brdf_lut, brdf_lut_sampler);
    float3 F0 = base_reflectance(surface.base_color, surface.metallic);
    
    return radiance * (F0 * brdf.x + brdf.y);
}

float3 evaluate_diffuse_ibl(
    PbrSurface surface,
    TextureCube<half4> irradiance_map,
    SamplerState environment_sampler)
{
    float3 N = normalize(surface.normal);
    float3 V = normalize(surface.view_direction);
    
    float NdotV = saturate(dot(N, V));

    float3 F0 = base_reflectance(surface.base_color, surface.metallic);
    float3 F = fresnel_roughness(NdotV, F0, surface.roughness);

    float3 irradiance = irradiance_map.SampleLevel(environment_sampler, N, 0.0f).rgb;

    float3 kD = energy_conservation(F, surface.metallic);
    return kD * surface.base_color * irradiance;
}

#endif // OMEGA_PBR_IBL_HLSLI