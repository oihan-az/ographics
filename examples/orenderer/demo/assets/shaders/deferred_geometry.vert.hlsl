struct VSInput
{
    float3 position : POSITION;
    float4 normal : NORMAL;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

struct Constants
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
    float4 color;
};

[[vk::push_constant]]
ConstantBuffer<Constants> constants;

VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float4 world_position = mul(constants.model, float4(input.position.xyz, 1.0));
    float4 view_position = mul(constants.view, world_position);
    float4 clip_position = mul(constants.projection, view_position);

    output.position = clip_position;
    output.normal = normalize(input.normal.xyz);
    output.color = constants.color;

    return output;
}