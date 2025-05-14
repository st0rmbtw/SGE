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
    float4 color : Color;
    uint flags : Flags;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 color : Color;
};

static const uint FLAG_UI = 1 << 0;

VSOutput VS(VSInput inp)
{
    const bool is_ui = (inp.flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = is_ui ? u_screen_projection : u_view_projection;

    VSOutput outp;
    outp.color = inp.color;
    outp.position = mul(mvp, float4(inp.position, 0.0, 1.0));
    outp.position.z = 1.0f;

	return outp;
}

float4 PS(VSOutput inp) : SV_Target
{
    clip(inp.color.a - 0.05f);

    return inp.color;
};

