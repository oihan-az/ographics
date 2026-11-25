#ifndef OMEGA_PBR_ENERGY_HLSLI
#define OMEGA_PBR_ENERGY_HLSLI

float3 energy_conservation(float3 kS, float metallic)
{
    return (1.0f - kS) * (1.0f - metallic);
}

#endif // OMEGA_PBR_ENERGY_HLSLI