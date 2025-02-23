struct VSInput
{
    float2 position : Position;
    float2 uv : UV;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : UV;
};

VSOutput VS(VSInput inp)
{
    VSOutput outp;
    outp.uv = inp.uv;
    outp.position = float4(inp.position, 0.0, 1.0);

	return outp;
}

Texture2D Texture : register(t2);
SamplerState Sampler : register(s3);

float4 PS(VSOutput inp) : SV_Target
{
    return Texture.Sample(Sampler, inp.uv);
};

