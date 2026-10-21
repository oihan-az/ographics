struct VSInput
{
    float3 position : POSITION;
    float3 color : COLOR0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
    float t : TIME;
};

[[vk::binding(0, 0)]]
cbuffer TransformBuffer
{
    float4x4 model2w;
    float t;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;

    output.position = mul(model2w, float4(input.position, 1.0));
    output.color = input.color;
    output.t = t;

    return output;
}
