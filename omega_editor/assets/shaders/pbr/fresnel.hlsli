#ifndef OMEGA_PBR_FRESNEL_HLSLI
#define OMEGA_PBR_FRESNEL_HLSLI

float3 fresnel_schlick(float cos_theta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - saturate(cos_theta), 5.0f);
}

#endif // OMEGA_PBR_FRESNEL_HLSLI