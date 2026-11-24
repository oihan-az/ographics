[[vk::binding(0, 0)]]
TextureCube<float4> environment_map;

[[vk::binding(1, 0)]]
SamplerState environment_sampler;

struct PSInput
{
    float4 position : SV_Position;
    float3 direction : TEXCOORD0;
};

float4 PSMain(PSInput input) : SV_Target0
{
    float3 direction = normalize(input.direction);
    float3 radiance = environment_map.SampleLevel(environment_sampler, direction, 0.0f).rgb;
    return float4(radiance, 1.0f);
}
