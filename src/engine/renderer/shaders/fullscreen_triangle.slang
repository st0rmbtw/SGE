struct VSInput
{
    float2 position : Position;
    float2 uv : UV;
};

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : UV;
};

[shader("vertex")]
VSOutput VS(VSInput inp)
{
    VSOutput output;
    output.uv = inp.uv;
    output.position = float4(inp.position, 0.0, 1.0);
    return output;
}

[vk::binding(2)]
Texture2D SourceTexture : register(t2);

[vk::binding(3)]
SamplerState SourceSampler : register(s3);

[shader("fragment")]
float4 PS(VSOutput inp) : SV_Target {
    return SourceTexture.SampleLevel(SourceSampler, inp.uv, 0);
}