struct PSInput
{
    float4 position : SV_Position;
};

float4 PSMain(PSInput input) : SV_Target
{
    return float4(1.0, 0.0, 0.0, 1.0);
}