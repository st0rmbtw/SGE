[vk::binding(2)]
cbuffer GlobalUniformBuffer : register( b2 )
{
    float4x4 u_screen_projection;
    float4x4 u_view_projection;
    float4x4 u_nonscale_view_projection;
    float4x4 u_nonscale_projection;
    float4x4 u_transform_matrix;
    float4x4 u_inv_view_proj;
    float2 u_camera_position;
    float2 u_window_size;
};

struct VSInput
{
    float2 position : Position;

    float3 i_color : I_Color;
    float2 i_position : I_Position;
    float2 i_size : I_Size;
    float2 i_tex_size : I_TexSize;
    float2 i_uv : I_UV;
    uint i_flags : I_Flags;
};

struct VSOutput
{
    float2 uv : UV;
    float4 position : SV_Position;
};

static const uint FLAG_UI = 1 << 0;

[shader("vertex")]
VSOutput VS(VSInput inp)
{
    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = is_ui ? u_screen_projection : u_view_projection;

    const float2 position = inp.i_position + inp.position * inp.i_size;
    const float2 uv = inp.i_uv + inp.position * inp.i_tex_size;

    VSOutput outp;
    outp.color = inp.i_color;
    outp.uv = uv;
    outp.tex_size = inp.i_tex_size;
    outp.position = mul(mvp, float4(position, 0.0, 1.0));
    outp.position.z = 1.0f;

	return outp;
}

[vk::binding(3)]
Texture2D Texture : register(t3);

[vk::binding(4)]
SamplerState Sampler : register(s4);

static const float GLYPH_CENTER = 0.5;

[shader("fragment")]
float4 PS(VSOutput inp) : SV_Target
{
    const float dist = Texture.Sample(Sampler, inp.uv).r;
    const float width = fwidth(dist);
    const float alpha = smoothstep(GLYPH_CENTER - width, GLYPH_CENTER + width, abs(dist));
    const float4 color = float4(inp.color, alpha);

    if (color.a <= 0.05f) discard;

    return color;
};

