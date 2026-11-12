#include "lighting/lighting.hlsli"

struct PSInput
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float3 world_position : POSITION1;
};

struct PSOutput
{
    float4 albedo : SV_TARGET0;
};

[[vk::binding(0, 0)]]
StructuredBuffer<DirectionalLight> directional_lights;

[[vk::binding(1, 0)]]
StructuredBuffer<PointLight> point_lights;

[[vk::binding(2, 0)]]
StructuredBuffer<SpotLight> spot_lights;

struct LightingConstants
{
    [[vk::offset(144)]]
    uint directional_light_count;

    uint point_light_count;
    uint spot_light_count;
    uint padding;
};

[[vk::push_constant]]
ConstantBuffer<LightingConstants> lighting_constants;

PSOutput PSMain(PSInput input)
{
    PSOutput output;

    LightingSurface surface;
    surface.position = input.world_position;
    surface.normal = normalize(input.normal);

    float3 lighting = evaluate_lighting(
        surface,
        directional_lights,
        lighting_constants.directional_light_count,
        point_lights,
        lighting_constants.point_light_count,
        spot_lights,
        lighting_constants.spot_light_count
    );

    output.albedo = float4(input.color.rgb * lighting, input.color.a);

    return output;
}