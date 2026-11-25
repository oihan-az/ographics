#ifndef OMEGA_BRDF_LUT_COMP_HLSL
#define OMEGA_BRDF_LUT_COMP_HLSL

#include "geometry.hlsli"
#include "importance_sampling.hlsli"

static const uint BRDF_SAMPLE_COUNT = 1024;

[[vk::binding(0, 0)]]
RWTexture2D<half2> brdf_lut;

float2 integrate_brdf(float NdotV, float roughness)
{
    float3 N = float3(0.0f, 0.0f, 1.0f);
    float3 V = float3(sqrt(max(1.0f - NdotV * NdotV, 0.0f)), 0.0f, NdotV);

    float A = 0.0f;
    float B = 0.0f;
    for (uint i = 0; i < BRDF_SAMPLE_COUNT; ++i)
    {
        float2 xi = hammersley(i, BRDF_SAMPLE_COUNT);

        float3 H = importance_sample_ggx(xi, N, roughness);
        float VdotH = saturate(dot(V, H));
        float3 L = normalize(2.0f * VdotH * H - V);

        float NdotL = saturate(L.z);
        float NdotH = saturate(H.z);
        if (NdotL > 0.0f)
        {
            float G = geometry_smith_ibl(NdotV, NdotL, roughness);
            float G_vis = (G * VdotH) / max(NdotH * NdotV, 1e-6f);

            float Fc = pow(1.0f - VdotH, 5.0f);

            A += (1.0f - Fc) * G_vis;
            B += Fc * G_vis;
        }
    }

    return float2(A, B) / float(BRDF_SAMPLE_COUNT);
}

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatch_id : SV_DispatchThreadID)
{
    uint width, height;
    brdf_lut.GetDimensions(width, height);
    if (dispatch_id.x >= width || dispatch_id.y >= height)
        return;

    float2 uv = (float2(dispatch_id.xy) + 0.5f) / float2(width, height);

    float NdotV = max(uv.x, 1e-4f);
    float roughness = max(uv.y, 1e-4f);

    brdf_lut[dispatch_id.xy] = integrate_brdf(NdotV, roughness);
}

#endif // OMEGA_BRDF_LUT_COMP_HLSL