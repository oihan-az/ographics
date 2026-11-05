struct PSInput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

struct PSOutput
{
    float4 albedo : SV_TARGET0;
    float4 normal : SV_TARGET1;
};

PSOutput PSMain(PSInput input)
{
    PSOutput output;

    output.albedo = input.color;
    output.normal = float4(normalize(input.normal), 1.0);

    return output;
}