#ifndef OMEGA_PBR_IBL_HLSLI
#define OMEGA_PBR_IBL_HLSLI

// Analytic approximation of the split-sum environment BRDF.
// Returns the scale and bias applied to F0.
float2 environment_brdf(float NdotV, float roughness)
{
    const float4 c0 = float4(-1.0f, -0.0275f, -0.572f, 0.022f);
    const float4 c1 = float4(1.0f, 0.0425f, 1.040f, -0.040f);

    float4 r = roughness * c0 + c1;

    float a004 = min(r.x * r.x, exp2(-9.28f * NdotV)) * r.x + r.y;

    return float2(-1.04f, 1.04f) * a004 + r.zw;
}

float3 evaluate_specular_ibl(
    float3 normal,
    float3 view_direction,
    float3 base_color,
    float metallic,
    float roughness,
    TextureCube<float4> environment_map,
    SamplerState environment_sampler)
{
    float3 N = normalize(normal);
    float3 V = normalize(view_direction);
    float3 R = reflect(-V, N);

    uint width, height, mip_count;
    environment_map.GetDimensions(0, width, height, mip_count);
    float lod = roughness * float(mip_count - 1);

    float3 radiance = environment_map.SampleLevel(environment_sampler, R, lod).rgb;
    float NdotV = saturate(dot(N, V));
    float2 brdf = environment_brdf(NdotV, roughness);
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), base_color, saturate(metallic));
    
    return radiance * (F0 * brdf.x + brdf.y);
}

#endif // OMEGA_PBR_IBL_HLSLI