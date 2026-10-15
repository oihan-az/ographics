
struct VSInput
{
    uint vertex_id : SV_VertexID;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;

    const float3 positions[3] =
    {
        float3(0.0, -0.5, 0.0),
        float3(0.5, 0.5, 0.0),
        float3(-0.5, 0.5, 0.0)
    };

    const float3 colors[3] =
    {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 0.0),
        float3(0.0, 0.0, 1.0)
    };

    output.position = float4(positions[input.vertex_id], 1.0);
    output.color = colors[input.vertex_id];

    return output;
}
