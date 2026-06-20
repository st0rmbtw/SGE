struct VSOutput {
    float4 position : SV_Position;
    float2 uv : UV;
};

[vk::binding(4)]
Texture2D MainTexture : register(t4);

[vk::binding(5)]
SamplerState MainSampler : register(s5);

float4 DownsampleBox13Tap(float2 uv, float2 texelSize)
{
    float4 A = MainTexture.Sample(MainSampler, uv + texelSize * float2(-1.0, -1.0));
    float4 B = MainTexture.Sample(MainSampler, uv + texelSize * float2( 0.0, -1.0));
    float4 C = MainTexture.Sample(MainSampler, uv + texelSize * float2( 1.0, -1.0));
    float4 D = MainTexture.Sample(MainSampler, uv + texelSize * float2(-0.5, -0.5));
    float4 E = MainTexture.Sample(MainSampler, uv + texelSize * float2( 0.5, -0.5));
    float4 F = MainTexture.Sample(MainSampler, uv + texelSize * float2(-1.0,  0.0));
    float4 G = MainTexture.Sample(MainSampler, uv                                 );
    float4 H = MainTexture.Sample(MainSampler, uv + texelSize * float2( 1.0,  0.0));
    float4 I = MainTexture.Sample(MainSampler, uv + texelSize * float2(-0.5,  0.5));
    float4 J = MainTexture.Sample(MainSampler, uv + texelSize * float2( 0.5,  0.5));
    float4 K = MainTexture.Sample(MainSampler, uv + texelSize * float2(-1.0,  1.0));
    float4 L = MainTexture.Sample(MainSampler, uv + texelSize * float2( 0.0,  1.0));
    float4 M = MainTexture.Sample(MainSampler, uv + texelSize * float2( 1.0,  1.0));

    float2 div = (1.0 / 4.0) * float2(0.5, 0.125);

    float4 o = (D + E + I + J) * div.x;
    o += (A + B + G + F) * div.y;
    o += (B + C + H + G) * div.y;
    o += (F + G + L + K) * div.y;
    o += (G + H + M + L) * div.y;

    return o;
}

[shader("fragment")]
float4 PS(VSOutput inp) : SV_Target
{
    float width;
    float height;
    MainTexture.GetDimensions(width, height);

    float2 texelSize = 1.0 / float2(width, height);

    return DownsampleBox13Tap(inp.uv, texelSize);
}