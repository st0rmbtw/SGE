// cbuffer GlobalUniformBuffer : register( b2 )
// {
//     float4x4 u_screen_projection;
//     float4x4 u_view_projection;
//     float4x4 u_nonscale_view_projection;
//     float4x4 u_nonscale_projection;
//     float4x4 u_transform_matrix;
//     float4x4 u_inv_view_proj;
//     float2 u_camera_position;
//     float2 u_window_size;
// };

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

uint uniform_blur_size;
uint uniform_direction;

VSOutput VS(VSInput inp)
{
    VSOutput outp;
    outp.uv = inp.uv;
    outp.position = float4(inp.position, 0.0, 1.0);

	return outp;
}

Texture2D Texture : register(t3);
SamplerState Sampler : register(s4);

static const float PI = 3.14159265;

float normpdf(in float x, in float sigma)
{
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

float gauss(float x, float sigma)
{
	return 1.0f / (2.0f * PI * sigma * sigma) * exp(-(x * x) / (2.0f * sigma * sigma));
}

float4 PS(VSOutput inp) : SV_Target
{
    float width;
    float height;
    Texture.GetDimensions(width, height);

    float2 size = float2(width, height);

    float2 direction = uniform_direction == 0 ? float2(1.0, 0.0) : float2(0.0, 1.0);

    float3 result = float3(0.0, 0.0, 0.0);
    float sum = 0.0;
    float sigma = 4.0f;
    
    int kSize = int(uniform_blur_size / 2);

    for (int i = -kSize; i <= kSize; ++i)
    {
        float2 offset = direction * (float(i) + 0.5f) / size;
        float weight = gauss(float(i), sigma);
        // float weight = normpdf(float(i), sigma);

        result += weight * Texture.Sample(Sampler, inp.uv + offset).rgb;
        sum += weight;
    }
    
    return float4(result / sum, 1.0);
};

