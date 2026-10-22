struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};


struct Constants
{
    float time;
    float scale;
    float intensity;
    float color_phase;
};

[[vk::push_constant]]
ConstantBuffer<Constants> data;

float4 PSMain(PSInput input) : SV_TARGET
{
    float2 uv = (input.uv - 0.5) * data.scale;

    float t = data.time;

    float v =
        sin(uv.x * 3.0 + t) +
        sin(uv.y * 4.0 - t * 1.3) +
        sin((uv.x + uv.y) * 3.0 + t * 0.7) +
        sin(length(uv) * 5.0 - t * 1.5);

    float3 color;
    color.r = sin(v + data.color_phase);
    color.g = sin(v + 2.0 + data.color_phase);
    color.b = sin(v + 4.0 + data.color_phase);

    color = 0.5 + 0.5 * color;
    color *= data.intensity;

    return float4(color, 1.0);
}