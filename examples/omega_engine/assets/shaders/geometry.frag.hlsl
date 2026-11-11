struct PSInput
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

struct PSOutput
{
    float4 albedo : SV_TARGET0;
};

PSOutput PSMain(PSInput input)
{
    PSOutput output;

    output.albedo = input.color;

    return output;
}