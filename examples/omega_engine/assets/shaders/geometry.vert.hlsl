struct VSInput
{
    float3 position : POSITION;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

struct Constants
{
    float4x4 model;
    float4x4 view_projection;
    float4 color;
};

[[vk::push_constant]]
ConstantBuffer<Constants> constants;

VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float4 world_position = mul(constants.model, float4(input.position, 1.0));
    output.position = mul(constants.view_projection, world_position);

    output.color = constants.color;

    return output;
}