struct ShadowDrawConstants
{
    float4x4 model;
    float4x4 light_view_projection;
};

[[vk::push_constant]] ConstantBuffer<ShadowDrawConstants> constants;

struct VSInput
{
    [[vk::location(0)]] float3 position : POSITION;
};

float4 VSMain(VSInput input) : SV_Position
{
    const float4 world_position = mul(constants.model, float4(input.position, 1.0f));
    return mul(constants.light_view_projection, world_position);
}
