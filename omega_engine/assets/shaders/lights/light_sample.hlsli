#ifndef OMEGA_LIGHT_SAMPLE_HLSLI
#define OMEGA_LIGHT_SAMPLE_HLSLI

struct LightSample
{
    // Normalized direction from the surface toward the light
    float3 direction;

    // Incident radiance, including intensity and attenuation
    float3 radiance;
};

#endif // OMEGA_LIGHT_SAMPLE_HLSLI