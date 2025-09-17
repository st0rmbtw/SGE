#ifndef _SGE_RENDERER_SHADERS_HPP_
#define _SGE_RENDERER_SHADERS_HPP_

#include <cstdlib>
#include <SGE/types/backend.hpp>
#include <SGE/assert.hpp>

static const char D3D11_FONT_VERT[1518] = R"(#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#ifndef __DXC_VERSION_MAJOR
#pragma warning(disable : 3557)
#endif
struct GlobalUniforms_0
{
float4x4 screen_projection_0;
float4x4 view_projection_0;
float4x4 nonscale_view_projection_0;
float4x4 nonscale_projection_0;
float4x4 transform_matrix_0;
float4x4 inv_view_proj_0;
float2 camera_position_0;
float2 window_size_0;
};
struct SLANG_ParameterGroup_GlobalUniformBuffer_0
{
GlobalUniforms_0 uniforms_0;
};
cbuffer GlobalUniformBuffer_0 : register(b2)
{
SLANG_ParameterGroup_GlobalUniformBuffer_0 GlobalUniformBuffer_0;
}
struct VSOutput_0
{
float4 position_0 : SV_Position;
float2 uv_0 : UV;
nointerpolation float3 color_0 : Color;
};
struct VSInput_0
{
float2 position_1 : Position;
float3 i_color_0 : I_Color;
float2 i_position_0 : I_Position;
float2 i_size_0 : I_Size;
float2 i_tex_size_0 : I_TexSize;
float2 i_uv_0 : I_UV;
uint i_flags_0 : I_Flags;
};
VSOutput_0 VS(VSInput_0 inp_0)
{
VSInput_0 _S1 = inp_0;
float4x4 mvp_0;
if(((inp_0.i_flags_0) & 1U) == 1U)
{
mvp_0 = GlobalUniformBuffer_0.uniforms_0.screen_projection_0;
}
else
{
mvp_0 = GlobalUniformBuffer_0.uniforms_0.view_projection_0;
}
float2 position_2 = _S1.i_position_0 + _S1.position_1 * _S1.i_size_0;
float2 uv_1 = _S1.i_uv_0 + _S1.position_1 * _S1.i_tex_size_0;
VSOutput_0 outp_0;
outp_0.color_0 = _S1.i_color_0;
outp_0.uv_0 = uv_1;
outp_0.position_0 = mul(mvp_0, float4(position_2, 0.0f, 1.0f));
outp_0.position_0[int(2)] = 1.0f;
return outp_0;
}
)";

static const char D3D11_FONT_FRAG[667] = R"(#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#ifndef __DXC_VERSION_MAJOR
#pragma warning(disable : 3557)
#endif
Texture2D<float4 > Texture_0 : register(t3);
SamplerState Sampler_0 : register(s4);
struct VSOutput_0
{
float4 position_0 : SV_Position;
float2 uv_0 : UV;
nointerpolation float3 color_0 : Color;
};
float4 PS(VSOutput_0 inp_0) : SV_TARGET
{
float dist_0 = Texture_0.Sample(Sampler_0, inp_0.uv_0).x;
float width_0 = (fwidth((dist_0)));
float4 color_1 = float4(inp_0.color_0, smoothstep(0.5f - width_0, 0.5f + width_0, abs(dist_0)));
if((color_1.w) <= 0.05000000074505806f)
{
discard;
}
return color_1;
}
)";

static const unsigned char VULKAN_FONT_VERT[3564] = {3, 2, 35, 7, 0, 5, 1, 0, 0, 0, 40, 0, 118, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 16, 0, 0, 0, 0, 0, 1, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 10, 0, 0, 0, 11, 0, 0, 0, 12, 0, 0, 0, 3, 0, 3, 0, 11, 0, 0, 0, 1, 0, 0, 0, 5, 0, 6, 0, 6, 0, 0, 0, 105, 110, 112, 46, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 0, 0, 5, 0, 5, 0, 7, 0, 0, 0, 105, 110, 112, 46, 105, 95, 99, 111, 108, 111, 114, 0, 5, 0, 6, 0, 8, 0, 0, 0, 105, 110, 112, 46, 105, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 5, 0, 9, 0, 0, 0, 105, 110, 112, 46, 105, 95, 115, 105, 122, 101, 0, 0, 5, 0, 6, 0, 10, 0, 0, 0, 105, 110, 112, 46, 105, 95, 116, 101, 120, 95, 115, 105, 122, 101, 0, 0, 5, 0, 5, 0, 11, 0, 0, 0, 105, 110, 112, 46, 105, 95, 117, 118, 0, 0, 0, 0, 5, 0, 5, 0, 12, 0, 0, 0, 105, 110, 112, 46, 105, 95, 102, 108, 97, 103, 115, 0, 5, 0, 4, 0, 13, 0, 0, 0, 105, 115, 95, 117, 105, 0, 0, 0, 5, 0, 12, 0, 14, 0, 0, 0, 95, 77, 97, 116, 114, 105, 120, 83, 116, 111, 114, 97, 103, 101, 95, 102, 108, 111, 97, 116, 52, 120, 52, 95, 67, 111, 108, 77, 97, 106, 111, 114, 115, 116, 100, 49, 52, 48, 0, 0, 6, 0, 5, 0, 14, 0, 0, 0, 0, 0, 0, 0, 100, 97, 116, 97, 0, 0, 0, 0, 5, 0, 8, 0, 15, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 115, 95, 115, 116, 100, 49, 52, 48, 0, 0, 0, 6, 0, 8, 0, 15, 0, 0, 0, 0, 0, 0, 0, 115, 99, 114, 101, 101, 110, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 7, 0, 15, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 10, 0, 15, 0, 0, 0, 2, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 0, 6, 0, 8, 0, 15, 0, 0, 0, 3, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 8, 0, 15, 0, 0, 0, 4, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 95, 109, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 7, 0, 15, 0, 0, 0, 5, 0, 0, 0, 105, 110, 118, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 0, 0, 0, 6, 0, 7, 0, 15, 0, 0, 0, 6, 0, 0, 0, 99, 97, 109, 101, 114, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 6, 0, 15, 0, 0, 0, 7, 0, 0, 0, 119, 105, 110, 100, 111, 119, 95, 115, 105, 122, 101, 0, 5, 0, 14, 0, 16, 0, 0, 0, 83, 76, 65, 78, 71, 95, 80, 97, 114, 97, 109, 101, 116, 101, 114, 71, 114, 111, 117, 112, 95, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 95, 115, 116, 100, 49, 52, 48, 0, 6, 0, 6, 0, 16, 0, 0, 0, 0, 0, 0, 0, 117, 110, 105, 102, 111, 114, 109, 115, 0, 0, 0, 0, 5, 0, 7, 0, 2, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 5, 0, 5, 0, 17, 0, 0, 0, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 0, 0, 5, 0, 3, 0, 18, 0, 0, 0, 117, 118, 0, 0, 5, 0, 8, 0, 4, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 117, 118, 0, 0, 0, 5, 0, 9, 0, 5, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 99, 111, 108, 111, 114, 0, 0, 0, 0, 5, 0, 3, 0, 1, 0, 0, 0, 86, 83, 0, 0, 71, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 7, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 8, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 9, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 10, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 4, 0, 12, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 4, 0, 19, 0, 0, 0, 6, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 14, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 15, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 15, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 5, 0, 15, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 72, 0, 5, 0, 15, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 192, 0, 0, 0, 72, 0, 5, 0, 15, 0, 0, 0, 4, 0, 0, 0, 35, 0, 0, 0, 0, 1, 0, 0, 72, 0, 5, 0, 15, 0, 0, 0, 5, 0, 0, 0, 35, 0, 0, 0, 64, 1, 0, 0, 72, 0, 5, 0, 15, 0, 0, 0, 6, 0, 0, 0, 35, 0, 0, 0, 128, 1, 0, 0, 72, 0, 5, 0, 15, 0, 0, 0, 7, 0, 0, 0, 35, 0, 0, 0, 136, 1, 0, 0, 71, 0, 3, 0, 16, 0, 0, 0, 2, 0, 0, 0, 72, 0, 5, 0, 16, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 2, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 2, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 4, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 5, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 3, 0, 5, 0, 0, 0, 14, 0, 0, 0, 19, 0, 2, 0, 20, 0, 0, 0, 33, 0, 3, 0, 21, 0, 0, 0, 20, 0, 0, 0, 22, 0, 3, 0, 22, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 23, 0, 0, 0, 22, 0, 0, 0, 4, 0, 0, 0, 24, 0, 4, 0, 24, 0, 0, 0, 23, 0, 0, 0, 4, 0, 0, 0, 23, 0, 4, 0, 25, 0, 0, 0, 22, 0, 0, 0, 2, 0, 0, 0, 23, 0, 4, 0, 26, 0, 0, 0, 22, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 27, 0, 0, 0, 1, 0, 0, 0, 25, 0, 0, 0, 32, 0, 4, 0, 28, 0, 0, 0, 1, 0, 0, 0, 26, 0, 0, 0, 21, 0, 4, 0, 29, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 30, 0, 0, 0, 1, 0, 0, 0, 29, 0, 0, 0, 43, 0, 4, 0, 29, 0, 0, 0, 31, 0, 0, 0, 1, 0, 0, 0, 20, 0, 2, 0, 32, 0, 0, 0, 21, 0, 4, 0, 33, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 33, 0, 0, 0, 34, 0, 0, 0, 4, 0, 0, 0, 28, 0, 4, 0, 19, 0, 0, 0, 23, 0, 0, 0, 34, 0, 0, 0, 30, 0, 3, 0, 14, 0, 0, 0, 19, 0, 0, 0, 30, 0, 10, 0, 15, 0, 0, 0, 14, 0, 0, 0, 14, 0, 0, 0, 14, 0, 0, 0, 14, 0, 0, 0, 14, 0, 0, 0, 14, 0, 0, 0, 25, 0, 0, 0, 25, 0, 0, 0, 30, 0, 3, 0, 16, 0, 0, 0, 15, 0, 0, 0, 32, 0, 4, 0, 35, 0, 0, 0, 2, 0, 0, 0, 16, 0, 0, 0, 43, 0, 4, 0, 33, 0, 0, 0, 36, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 33, 0, 0, 0, 37, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 38, 0, 0, 0, 2, 0, 0, 0, 14, 0, 0, 0, 43, 0, 4, 0, 22, 0, 0, 0, 39, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 22, 0, 0, 0, 40, 0, 0, 0, 0, 0, 128, 63, 32, 0, 4, 0, 41, 0, 0, 0, 3, 0, 0, 0, 23, 0, 0, 0, 32, 0, 4, 0, 42, 0, 0, 0, 3, 0, 0, 0, 25, 0, 0, 0, 32, 0, 4, 0, 43, 0, 0, 0, 3, 0, 0, 0, 26, 0, 0, 0, 59, 0, 4, 0, 27, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 7, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 27, 0, 0, 0, 8, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 27, 0, 0, 0, 9, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 27, 0, 0, 0, 10, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 27, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 30, 0, 0, 0, 12, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 35, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 59, 0, 4, 0, 41, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 42, 0, 0, 0, 4, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 43, 0, 0, 0, 5, 0, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 20, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 21, 0, 0, 0, 248, 0, 2, 0, 44, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 45, 0, 0, 0, 6, 0, 0, 0, 61, 0, 4, 0, 26, 0, 0, 0, 46, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 47, 0, 0, 0, 8, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 48, 0, 0, 0, 9, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 49, 0, 0, 0, 10, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 50, 0, 0, 0, 11, 0, 0, 0, 61, 0, 4, 0, 29, 0, 0, 0, 51, 0, 0, 0, 12, 0, 0, 0, 199, 0, 5, 0, 29, 0, 0, 0, 52, 0, 0, 0, 51, 0, 0, 0, 31, 0, 0, 0, 170, 0, 5, 0, 32, 0, 0, 0, 13, 0, 0, 0, 52, 0, 0, 0, 31, 0, 0, 0, 247, 0, 3, 0, 53, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 13, 0, 0, 0, 54, 0, 0, 0, 55, 0, 0, 0, 248, 0, 2, 0, 55, 0, 0, 0, 66, 0, 6, 0, 38, 0, 0, 0, 56, 0, 0, 0, 2, 0, 0, 0, 36, 0, 0, 0, 37, 0, 0, 0, 61, 0, 4, 0, 14, 0, 0, 0, 57, 0, 0, 0, 56, 0, 0, 0, 81, 0, 5, 0, 19, 0, 0, 0, 58, 0, 0, 0, 57, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 59, 0, 0, 0, 58, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 60, 0, 0, 0, 59, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 61, 0, 0, 0, 59, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 62, 0, 0, 0, 59, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 63, 0, 0, 0, 59, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 64, 0, 0, 0, 58, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 65, 0, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 66, 0, 0, 0, 64, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 67, 0, 0, 0, 64, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 68, 0, 0, 0, 64, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 69, 0, 0, 0, 58, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 70, 0, 0, 0, 69, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 71, 0, 0, 0, 69, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 72, 0, 0, 0, 69, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 73, 0, 0, 0, 69, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 74, 0, 0, 0, 58, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 75, 0, 0, 0, 74, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 76, 0, 0, 0, 74, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 77, 0, 0, 0, 74, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 78, 0, 0, 0, 74, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 23, 0, 0, 0, 79, 0, 0, 0, 60, 0, 0, 0, 65, 0, 0, 0, 70, 0, 0, 0, 75, 0, 0, 0, 80, 0, 7, 0, 23, 0, 0, 0, 80, 0, 0, 0, 61, 0, 0, 0, 66, 0, 0, 0, 71, 0, 0, 0, 76, 0, 0, 0, 80, 0, 7, 0, 23, 0, 0, 0, 81, 0, 0, 0, 62, 0, 0, 0, 67, 0, 0, 0, 72, 0, 0, 0, 77, 0, 0, 0, 80, 0, 7, 0, 23, 0, 0, 0, 82, 0, 0, 0, 63, 0, 0, 0, 68, 0, 0, 0, 73, 0, 0, 0, 78, 0, 0, 0, 80, 0, 7, 0, 24, 0, 0, 0, 83, 0, 0, 0, 79, 0, 0, 0, 80, 0, 0, 0, 81, 0, 0, 0, 82, 0, 0, 0, 249, 0, 2, 0, 53, 0, 0, 0, 248, 0, 2, 0, 54, 0, 0, 0, 66, 0, 6, 0, 38, 0, 0, 0, 84, 0, 0, 0, 2, 0, 0, 0, 36, 0, 0, 0, 36, 0, 0, 0, 61, 0, 4, 0, 14, 0, 0, 0, 85, 0, 0, 0, 84, 0, 0, 0, 81, 0, 5, 0, 19, 0, 0, 0, 86, 0, 0, 0, 85, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 87, 0, 0, 0, 86, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 88, 0, 0, 0, 87, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 89, 0, 0, 0, 87, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 90, 0, 0, 0, 87, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 91, 0, 0, 0, 87, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 92, 0, 0, 0, 86, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 93, 0, 0, 0, 92, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 94, 0, 0, 0, 92, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 95, 0, 0, 0, 92, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 96, 0, 0, 0, 92, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 97, 0, 0, 0, 86, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 98, 0, 0, 0, 97, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 99, 0, 0, 0, 97, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 100, 0, 0, 0, 97, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 101, 0, 0, 0, 97, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 102, 0, 0, 0, 86, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 103, 0, 0, 0, 102, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 104, 0, 0, 0, 102, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 105, 0, 0, 0, 102, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 22, 0, 0, 0, 106, 0, 0, 0, 102, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 23, 0, 0, 0, 107, 0, 0, 0, 88, 0, 0, 0, 93, 0, 0, 0, 98, 0, 0, 0, 103, 0, 0, 0, 80, 0, 7, 0, 23, 0, 0, 0, 108, 0, 0, 0, 89, 0, 0, 0, 94, 0, 0, 0, 99, 0, 0, 0, 104, 0, 0, 0, 80, 0, 7, 0, 23, 0, 0, 0, 109, 0, 0, 0, 90, 0, 0, 0, 95, 0, 0, 0, 100, 0, 0, 0, 105, 0, 0, 0, 80, 0, 7, 0, 23, 0, 0, 0, 110, 0, 0, 0, 91, 0, 0, 0, 96, 0, 0, 0, 101, 0, 0, 0, 106, 0, 0, 0, 80, 0, 7, 0, 24, 0, 0, 0, 111, 0, 0, 0, 107, 0, 0, 0, 108, 0, 0, 0, 109, 0, 0, 0, 110, 0, 0, 0, 249, 0, 2, 0, 53, 0, 0, 0, 248, 0, 2, 0, 53, 0, 0, 0, 245, 0, 7, 0, 24, 0, 0, 0, 112, 0, 0, 0, 83, 0, 0, 0, 55, 0, 0, 0, 111, 0, 0, 0, 54, 0, 0, 0, 133, 0, 5, 0, 25, 0, 0, 0, 113, 0, 0, 0, 45, 0, 0, 0, 48, 0, 0, 0, 129, 0, 5, 0, 25, 0, 0, 0, 17, 0, 0, 0, 47, 0, 0, 0, 113, 0, 0, 0, 133, 0, 5, 0, 25, 0, 0, 0, 114, 0, 0, 0, 45, 0, 0, 0, 49, 0, 0, 0, 129, 0, 5, 0, 25, 0, 0, 0, 18, 0, 0, 0, 50, 0, 0, 0, 114, 0, 0, 0, 80, 0, 6, 0, 23, 0, 0, 0, 115, 0, 0, 0, 17, 0, 0, 0, 39, 0, 0, 0, 40, 0, 0, 0, 144, 0, 5, 0, 23, 0, 0, 0, 116, 0, 0, 0, 115, 0, 0, 0, 112, 0, 0, 0, 82, 0, 6, 0, 23, 0, 0, 0, 117, 0, 0, 0, 40, 0, 0, 0, 116, 0, 0, 0, 2, 0, 0, 0, 62, 0, 3, 0, 3, 0, 0, 0, 117, 0, 0, 0, 62, 0, 3, 0, 4, 0, 0, 0, 18, 0, 0, 0, 62, 0, 3, 0, 5, 0, 0, 0, 46, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_FONT_FRAG[1144] = {3, 2, 35, 7, 0, 5, 1, 0, 0, 0, 40, 0, 42, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 10, 0, 4, 0, 0, 0, 2, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 16, 0, 3, 0, 2, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 11, 0, 0, 0, 1, 0, 0, 0, 5, 0, 4, 0, 6, 0, 0, 0, 105, 110, 112, 46, 117, 118, 0, 0, 5, 0, 5, 0, 7, 0, 0, 0, 105, 110, 112, 46, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 4, 0, 3, 0, 0, 0, 84, 101, 120, 116, 117, 114, 101, 0, 5, 0, 4, 0, 4, 0, 0, 0, 83, 97, 109, 112, 108, 101, 114, 0, 5, 0, 6, 0, 8, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 73, 109, 97, 103, 101, 0, 0, 0, 0, 5, 0, 4, 0, 9, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 0, 5, 0, 4, 0, 10, 0, 0, 0, 100, 105, 115, 116, 0, 0, 0, 0, 5, 0, 4, 0, 11, 0, 0, 0, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 7, 0, 5, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 80, 83, 0, 0, 5, 0, 3, 0, 2, 0, 0, 0, 80, 83, 0, 0, 71, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 7, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 3, 0, 7, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 33, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 4, 0, 0, 0, 33, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 4, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 5, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 12, 0, 0, 0, 33, 0, 3, 0, 13, 0, 0, 0, 12, 0, 0, 0, 22, 0, 3, 0, 14, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 15, 0, 0, 0, 14, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 16, 0, 0, 0, 1, 0, 0, 0, 15, 0, 0, 0, 23, 0, 4, 0, 17, 0, 0, 0, 14, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 18, 0, 0, 0, 1, 0, 0, 0, 17, 0, 0, 0, 25, 0, 9, 0, 19, 0, 0, 0, 14, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 20, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 26, 0, 2, 0, 21, 0, 0, 0, 32, 0, 4, 0, 22, 0, 0, 0, 0, 0, 0, 0, 21, 0, 0, 0, 27, 0, 3, 0, 23, 0, 0, 0, 19, 0, 0, 0, 23, 0, 4, 0, 24, 0, 0, 0, 14, 0, 0, 0, 4, 0, 0, 0, 43, 0, 4, 0, 14, 0, 0, 0, 25, 0, 0, 0, 0, 0, 0, 63, 20, 0, 2, 0, 26, 0, 0, 0, 43, 0, 4, 0, 14, 0, 0, 0, 27, 0, 0, 0, 205, 204, 76, 61, 32, 0, 4, 0, 28, 0, 0, 0, 3, 0, 0, 0, 24, 0, 0, 0, 59, 0, 4, 0, 16, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 18, 0, 0, 0, 7, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 20, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 59, 0, 4, 0, 22, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 5, 0, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 12, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 248, 0, 2, 0, 29, 0, 0, 0, 61, 0, 4, 0, 15, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 61, 0, 4, 0, 17, 0, 0, 0, 31, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 19, 0, 0, 0, 32, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 21, 0, 0, 0, 33, 0, 0, 0, 4, 0, 0, 0, 86, 0, 5, 0, 23, 0, 0, 0, 8, 0, 0, 0, 32, 0, 0, 0, 33, 0, 0, 0, 87, 0, 6, 0, 24, 0, 0, 0, 9, 0, 0, 0, 8, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 14, 0, 0, 0, 10, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 209, 0, 4, 0, 14, 0, 0, 0, 34, 0, 0, 0, 10, 0, 0, 0, 131, 0, 5, 0, 14, 0, 0, 0, 35, 0, 0, 0, 25, 0, 0, 0, 34, 0, 0, 0, 129, 0, 5, 0, 14, 0, 0, 0, 36, 0, 0, 0, 25, 0, 0, 0, 34, 0, 0, 0, 12, 0, 6, 0, 14, 0, 0, 0, 37, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 10, 0, 0, 0, 12, 0, 8, 0, 14, 0, 0, 0, 38, 0, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 35, 0, 0, 0, 36, 0, 0, 0, 37, 0, 0, 0, 80, 0, 5, 0, 24, 0, 0, 0, 11, 0, 0, 0, 31, 0, 0, 0, 38, 0, 0, 0, 188, 0, 5, 0, 26, 0, 0, 0, 39, 0, 0, 0, 38, 0, 0, 0, 27, 0, 0, 0, 247, 0, 3, 0, 40, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 39, 0, 0, 0, 41, 0, 0, 0, 40, 0, 0, 0, 248, 0, 2, 0, 41, 0, 0, 0, 252, 0, 1, 0, 248, 0, 2, 0, 40, 0, 0, 0, 62, 0, 3, 0, 5, 0, 0, 0, 11, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const char METAL_FONT_VERT[5445] = R"(#include <metal_stdlib>
#include <metal_math>
#include <metal_texture>
using namespace metal;
struct VS_Result_0
{
float4 position_0 [[position]];
float2 uv_0 [[user(UV)]];
float3 color_0 [[user(COLOR)]];
};
struct vertexInput_0
{
float2 position_1 [[attribute(0)]];
float3 i_color_0 [[attribute(1)]];
float2 i_position_0 [[attribute(2)]];
float2 i_size_0 [[attribute(3)]];
float2 i_tex_size_0 [[attribute(4)]];
float2 i_uv_0 [[attribute(5)]];
uint i_flags_0 [[attribute(6)]];
};
struct _MatrixStorage_float4x4_ColMajornatural_0
{
array<float4, int(4)> data_0;
};
struct GlobalUniforms_natural_0
{
_MatrixStorage_float4x4_ColMajornatural_0 screen_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 view_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 nonscale_view_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 nonscale_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 transform_matrix_0;
_MatrixStorage_float4x4_ColMajornatural_0 inv_view_proj_0;
float2 camera_position_0;
float2 window_size_0;
};
struct SLANG_ParameterGroup_GlobalUniformBuffer_natural_0
{
GlobalUniforms_natural_0 uniforms_0;
};
struct KernelContext_0
{
SLANG_ParameterGroup_GlobalUniformBuffer_natural_0 constant* GlobalUniformBuffer_0;
};
struct VSOutput_0
{
float4 position_2;
float2 uv_1;
[[flat]] float3 color_1;
};
[[vertex]] VS_Result_0 VS(vertexInput_0 _S1 [[stage_in]], SLANG_ParameterGroup_GlobalUniformBuffer_natural_0 constant* GlobalUniformBuffer_1 [[buffer(2)]])
{
KernelContext_0 kernelContext_0;
(&kernelContext_0)->GlobalUniformBuffer_0 = GlobalUniformBuffer_1;
matrix<float,int(4),int(4)>  mvp_0;
if(((_S1.i_flags_0) & 1U) == 1U)
{
mvp_0 = matrix<float,int(4),int(4)> ((&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(3)]);
}
else
{
mvp_0 = matrix<float,int(4),int(4)> ((&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(3)]);
}
float2 position_3 = _S1.i_position_0 + _S1.position_1 * _S1.i_size_0;
float2 uv_2 = _S1.i_uv_0 + _S1.position_1 * _S1.i_tex_size_0;
thread VSOutput_0 outp_0;
(&outp_0)->color_1 = _S1.i_color_0;
(&outp_0)->uv_1 = uv_2;
(&outp_0)->position_2 = (((float4(position_3, 0.0, 1.0)) * (mvp_0)));
(&outp_0)->position_2.z = 1.0;
VSOutput_0 _S2 = outp_0;
thread VS_Result_0 _S3;
(&_S3)->position_0 = _S2.position_2;
(&_S3)->uv_0 = _S2.uv_1;
(&_S3)->color_0 = _S2.color_1;
return _S3;
}
)";

static const char METAL_FONT_FRAG[964] = R"(#include <metal_stdlib>
#include <metal_math>
#include <metal_texture>
using namespace metal;
struct pixelOutput_0
{
float4 output_0 [[color(0)]];
};
struct pixelInput_0
{
float2 uv_0 [[user(UV)]];
[[flat]] float3 color_0 [[user(COLOR)]];
};
struct KernelContext_0
{
texture2d<float, access::sample> Texture_0;
sampler Sampler_0;
};
[[fragment]] pixelOutput_0 PS(pixelInput_0 _S1 [[stage_in]], float4 position_0 [[position]], texture2d<float, access::sample> Texture_1 [[texture(3)]], sampler Sampler_1 [[sampler(4)]])
{
KernelContext_0 kernelContext_0;
(&kernelContext_0)->Texture_0 = Texture_1;
(&kernelContext_0)->Sampler_0 = Sampler_1;
float dist_0 = (((&kernelContext_0)->Texture_0).sample((Sampler_1), (_S1.uv_0))).x;
float width_0 = (fwidth((dist_0)));
float4 color_1 = float4(_S1.color_0, smoothstep(0.5 - width_0, 0.5 + width_0, abs(dist_0)));
if((color_1.w) <= 0.05000000074505806)
{
discard_fragment();
}
pixelOutput_0 _S2 = { color_1 };
return _S2;
}
)";

static const char GL_FONT_VERT[3212] = R"(#version 410
struct _MatrixStorage_float4x4_ColMajorstd140
{
vec4 data[4];
};
struct GlobalUniforms_std140
{
_MatrixStorage_float4x4_ColMajorstd140 screen_projection;
_MatrixStorage_float4x4_ColMajorstd140 view_projection;
_MatrixStorage_float4x4_ColMajorstd140 nonscale_view_projection;
_MatrixStorage_float4x4_ColMajorstd140 nonscale_projection;
_MatrixStorage_float4x4_ColMajorstd140 transform_matrix;
_MatrixStorage_float4x4_ColMajorstd140 inv_view_proj;
vec2 camera_position;
vec2 window_size;
};
layout(std140) uniform GlobalUniformBuffer_std140
{
GlobalUniforms_std140 uniforms;
} GlobalUniformBuffer;
layout(location = 0) in vec2 inp_position;
layout(location = 1) in vec3 inp_i_color;
layout(location = 2) in vec2 inp_i_position;
layout(location = 3) in vec2 inp_i_size;
layout(location = 4) in vec2 inp_i_tex_size;
layout(location = 5) in vec2 inp_i_uv;
layout(location = 6) in uint inp_i_flags;
layout(location = 0) out vec2 entryPointParam_VS_uv;
layout(location = 1) flat out vec3 entryPointParam_VS_color;
void main()
{
mat4 _112;
if ((inp_i_flags & 1u) == 1u)
{
_112 = mat4(vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].x, GlobalUniformBuffer.uniforms.screen_projection.data[1].x, GlobalUniformBuffer.uniforms.screen_projection.data[2].x, GlobalUniformBuffer.uniforms.screen_projection.data[3].x), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].y, GlobalUniformBuffer.uniforms.screen_projection.data[1].y, GlobalUniformBuffer.uniforms.screen_projection.data[2].y, GlobalUniformBuffer.uniforms.screen_projection.data[3].y), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].z, GlobalUniformBuffer.uniforms.screen_projection.data[1].z, GlobalUniformBuffer.uniforms.screen_projection.data[2].z, GlobalUniformBuffer.uniforms.screen_projection.data[3].z), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].w, GlobalUniformBuffer.uniforms.screen_projection.data[1].w, GlobalUniformBuffer.uniforms.screen_projection.data[2].w, GlobalUniformBuffer.uniforms.screen_projection.data[3].w));
}
else
{
_112 = mat4(vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].x, GlobalUniformBuffer.uniforms.view_projection.data[1].x, GlobalUniformBuffer.uniforms.view_projection.data[2].x, GlobalUniformBuffer.uniforms.view_projection.data[3].x), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].y, GlobalUniformBuffer.uniforms.view_projection.data[1].y, GlobalUniformBuffer.uniforms.view_projection.data[2].y, GlobalUniformBuffer.uniforms.view_projection.data[3].y), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].z, GlobalUniformBuffer.uniforms.view_projection.data[1].z, GlobalUniformBuffer.uniforms.view_projection.data[2].z, GlobalUniformBuffer.uniforms.view_projection.data[3].z), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].w, GlobalUniformBuffer.uniforms.view_projection.data[1].w, GlobalUniformBuffer.uniforms.view_projection.data[2].w, GlobalUniformBuffer.uniforms.view_projection.data[3].w));
}
vec4 _116 = vec4(inp_i_position + (inp_position * inp_i_size), 0.0, 1.0) * _112;
_116.z = 1.0;
gl_Position = _116;
entryPointParam_VS_uv = inp_i_uv + (inp_position * inp_i_tex_size);
entryPointParam_VS_color = inp_i_color;
}
)";

static const char GL_FONT_FRAG[436] = R"(#version 410
uniform sampler2D Texture;
layout(location = 0) in vec2 inp_uv;
layout(location = 1) flat in vec3 inp_color;
layout(location = 0) out vec4 entryPointParam_PS;
void main()
{
vec4 sampled = texture(Texture, inp_uv);
float dist = sampled.x;
float _34 = fwidth(dist);
float _38 = smoothstep(0.5 - _34, 0.5 + _34, abs(dist));
if (_38 <= 0.0500000007450580596923828125)
{
discard;
}
entryPointParam_PS = vec4(inp_color, _38);
}
)";

static const char D3D11_LINE_VERT[2000] = R"(#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#ifndef __DXC_VERSION_MAJOR
#pragma warning(disable : 3557)
#endif
struct GlobalUniforms_0
{
float4x4 screen_projection_0;
float4x4 view_projection_0;
float4x4 nonscale_view_projection_0;
float4x4 nonscale_projection_0;
float4x4 transform_matrix_0;
float4x4 inv_view_proj_0;
float2 camera_position_0;
float2 window_size_0;
};
struct SLANG_ParameterGroup_GlobalUniformBuffer_0
{
GlobalUniforms_0 uniforms_0;
};
cbuffer GlobalUniformBuffer_0 : register(b2)
{
SLANG_ParameterGroup_GlobalUniformBuffer_0 GlobalUniformBuffer_0;
}
struct VSOutput_0
{
float4 position_0 : SV_Position;
nointerpolation float4 color_0 : Color;
nointerpolation float4 border_radius_0 : BorderRadius;
nointerpolation float2 size_0 : Size;
float2 p_0 : Point;
float2 uv_0 : UV;
};
struct VSInput_0
{
float2 position_1 : Position;
float2 i_start_0 : I_Start;
float2 i_end_0 : I_End;
float4 i_color_0 : I_Color;
float4 i_border_radius_0 : I_Border_Radius;
float i_thickness_0 : I_Thickness;
uint i_flags_0 : I_Flags;
uint vid_0 : SV_VertexID;
};
VSOutput_0 VS(VSInput_0 inp_0)
{
VSInput_0 _S1 = inp_0;
float4x4 mvp_0;
if(((inp_0.i_flags_0) & 1U) == 1U)
{
mvp_0 = GlobalUniformBuffer_0.uniforms_0.screen_projection_0;
}
else
{
mvp_0 = GlobalUniformBuffer_0.uniforms_0.view_projection_0;
}
float2 d_0 = _S1.i_end_0 - _S1.i_start_0;
float len_0 = length(d_0);
float2 perp_0 = float2(d_0.y, - d_0.x) / len_0 * _S1.i_thickness_0 * 0.5f;
float2  vertices_0[int(4)] = { _S1.i_start_0 - perp_0, _S1.i_start_0 + perp_0, _S1.i_end_0 - perp_0, _S1.i_end_0 + perp_0 };
float2 size_1 = float2(len_0, _S1.i_thickness_0);
VSOutput_0 outp_0;
outp_0.color_0 = _S1.i_color_0;
outp_0.border_radius_0 = _S1.i_border_radius_0;
outp_0.size_0 = size_1;
outp_0.p_0 = (_S1.position_1 - 0.5f) * size_1;
outp_0.uv_0 = _S1.position_1;
outp_0.position_0 = mul(mvp_0, float4(vertices_0[_S1.vid_0], 0.0f, 1.0f));
outp_0.position_0[int(2)] = 1.0f;
return outp_0;
}
)";

static const char D3D11_LINE_FRAG[1354] = R"(#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#ifndef __DXC_VERSION_MAJOR
#pragma warning(disable : 3557)
#endif
float sd_rounded_box_0(float2 p_0, float2 size_0, float4 corner_radii_0)
{
float2 rs_0;
if(0.0f < (p_0.y))
{
rs_0 = corner_radii_0.zw;
}
else
{
rs_0 = corner_radii_0.xy;
}
float radius_0;
if(0.0f < (p_0.x))
{
radius_0 = rs_0.y;
}
else
{
radius_0 = rs_0.x;
}
float2 q_0 = abs(p_0) - 0.5f * size_0 + radius_0;
return length(max(q_0, float2(0.0f, 0.0f))) + min(max(q_0.x, q_0.y), 0.0f) - radius_0;
}
float antialias_0(float d_0)
{
return 1.0f - smoothstep(- pow((fwidth((d_0))), 0.45454543828964233f), 0.0f, d_0);
}
struct VSOutput_0
{
float4 position_0 : SV_Position;
nointerpolation float4 color_0 : Color;
nointerpolation float4 border_radius_0 : BorderRadius;
nointerpolation float2 size_1 : Size;
float2 p_1 : Point;
float2 uv_0 : UV;
};
float4 PS(VSOutput_0 inp_0) : SV_TARGET
{
VSOutput_0 _S1 = inp_0;
float4 result_0;
if((max(inp_0.border_radius_0.x, max(inp_0.border_radius_0.y, max(inp_0.border_radius_0.z, inp_0.border_radius_0.w)))) > 0.0f)
{
result_0 = float4(_S1.color_0.xyz, min(_S1.color_0.w, antialias_0(sd_rounded_box_0(_S1.p_1, _S1.size_1, _S1.border_radius_0))));
}
else
{
result_0 = _S1.color_0;
}
if((result_0.w) <= 0.05000000074505806f)
{
discard;
}
return result_0;
}
)";

static const unsigned char VULKAN_LINE_VERT[4484] = {3, 2, 35, 7, 0, 5, 1, 0, 0, 0, 40, 0, 150, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 75, 17, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 21, 0, 0, 0, 0, 0, 2, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 10, 0, 0, 0, 11, 0, 0, 0, 12, 0, 0, 0, 13, 0, 0, 0, 14, 0, 0, 0, 15, 0, 0, 0, 16, 0, 0, 0, 17, 0, 0, 0, 18, 0, 0, 0, 3, 0, 3, 0, 11, 0, 0, 0, 1, 0, 0, 0, 5, 0, 6, 0, 10, 0, 0, 0, 105, 110, 112, 46, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 0, 0, 5, 0, 5, 0, 11, 0, 0, 0, 105, 110, 112, 46, 105, 95, 115, 116, 97, 114, 116, 0, 5, 0, 5, 0, 12, 0, 0, 0, 105, 110, 112, 46, 105, 95, 101, 110, 100, 0, 0, 0, 5, 0, 5, 0, 13, 0, 0, 0, 105, 110, 112, 46, 105, 95, 99, 111, 108, 111, 114, 0, 5, 0, 7, 0, 14, 0, 0, 0, 105, 110, 112, 46, 105, 95, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 5, 0, 6, 0, 15, 0, 0, 0, 105, 110, 112, 46, 105, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 5, 0, 5, 0, 16, 0, 0, 0, 105, 110, 112, 46, 105, 95, 102, 108, 97, 103, 115, 0, 5, 0, 4, 0, 19, 0, 0, 0, 105, 115, 95, 117, 105, 0, 0, 0, 5, 0, 12, 0, 20, 0, 0, 0, 95, 77, 97, 116, 114, 105, 120, 83, 116, 111, 114, 97, 103, 101, 95, 102, 108, 111, 97, 116, 52, 120, 52, 95, 67, 111, 108, 77, 97, 106, 111, 114, 115, 116, 100, 49, 52, 48, 0, 0, 6, 0, 5, 0, 20, 0, 0, 0, 0, 0, 0, 0, 100, 97, 116, 97, 0, 0, 0, 0, 5, 0, 8, 0, 21, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 115, 95, 115, 116, 100, 49, 52, 48, 0, 0, 0, 6, 0, 8, 0, 21, 0, 0, 0, 0, 0, 0, 0, 115, 99, 114, 101, 101, 110, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 7, 0, 21, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 10, 0, 21, 0, 0, 0, 2, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 0, 6, 0, 8, 0, 21, 0, 0, 0, 3, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 8, 0, 21, 0, 0, 0, 4, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 95, 109, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 7, 0, 21, 0, 0, 0, 5, 0, 0, 0, 105, 110, 118, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 0, 0, 0, 6, 0, 7, 0, 21, 0, 0, 0, 6, 0, 0, 0, 99, 97, 109, 101, 114, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 6, 0, 21, 0, 0, 0, 7, 0, 0, 0, 119, 105, 110, 100, 111, 119, 95, 115, 105, 122, 101, 0, 5, 0, 14, 0, 22, 0, 0, 0, 83, 76, 65, 78, 71, 95, 80, 97, 114, 97, 109, 101, 116, 101, 114, 71, 114, 111, 117, 112, 95, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 95, 115, 116, 100, 49, 52, 48, 0, 6, 0, 6, 0, 22, 0, 0, 0, 0, 0, 0, 0, 117, 110, 105, 102, 111, 114, 109, 115, 0, 0, 0, 0, 5, 0, 7, 0, 3, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 5, 0, 3, 0, 23, 0, 0, 0, 100, 0, 0, 0, 5, 0, 4, 0, 24, 0, 0, 0, 112, 101, 114, 112, 0, 0, 0, 0, 5, 0, 5, 0, 25, 0, 0, 0, 118, 101, 114, 116, 105, 99, 101, 115, 0, 0, 0, 0, 5, 0, 4, 0, 26, 0, 0, 0, 115, 105, 122, 101, 0, 0, 0, 0, 5, 0, 9, 0, 5, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 99, 111, 108, 111, 114, 0, 0, 0, 0, 5, 0, 11, 0, 6, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 0, 0, 0, 5, 0, 8, 0, 7, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 115, 105, 122, 101, 0, 5, 0, 8, 0, 8, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 112, 0, 0, 0, 0, 5, 0, 8, 0, 9, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 117, 118, 0, 0, 0, 5, 0, 3, 0, 2, 0, 0, 0, 86, 83, 0, 0, 71, 0, 4, 0, 10, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 12, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 13, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 14, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 15, 0, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 4, 0, 16, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 4, 0, 18, 0, 0, 0, 11, 0, 0, 0, 72, 17, 0, 0, 71, 0, 4, 0, 17, 0, 0, 0, 11, 0, 0, 0, 42, 0, 0, 0, 71, 0, 4, 0, 27, 0, 0, 0, 6, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 20, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 21, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 21, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 5, 0, 21, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 72, 0, 5, 0, 21, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 192, 0, 0, 0, 72, 0, 5, 0, 21, 0, 0, 0, 4, 0, 0, 0, 35, 0, 0, 0, 0, 1, 0, 0, 72, 0, 5, 0, 21, 0, 0, 0, 5, 0, 0, 0, 35, 0, 0, 0, 64, 1, 0, 0, 72, 0, 5, 0, 21, 0, 0, 0, 6, 0, 0, 0, 35, 0, 0, 0, 128, 1, 0, 0, 72, 0, 5, 0, 21, 0, 0, 0, 7, 0, 0, 0, 35, 0, 0, 0, 136, 1, 0, 0, 71, 0, 3, 0, 22, 0, 0, 0, 2, 0, 0, 0, 72, 0, 5, 0, 22, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 4, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 5, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 5, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 3, 0, 6, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 7, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 3, 0, 7, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 8, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 9, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 19, 0, 2, 0, 28, 0, 0, 0, 33, 0, 3, 0, 29, 0, 0, 0, 28, 0, 0, 0, 22, 0, 3, 0, 30, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 31, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 24, 0, 4, 0, 32, 0, 0, 0, 31, 0, 0, 0, 4, 0, 0, 0, 23, 0, 4, 0, 33, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 21, 0, 4, 0, 34, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 34, 0, 0, 0, 35, 0, 0, 0, 4, 0, 0, 0, 28, 0, 4, 0, 36, 0, 0, 0, 33, 0, 0, 0, 35, 0, 0, 0, 32, 0, 4, 0, 37, 0, 0, 0, 7, 0, 0, 0, 36, 0, 0, 0, 32, 0, 4, 0, 38, 0, 0, 0, 1, 0, 0, 0, 33, 0, 0, 0, 32, 0, 4, 0, 39, 0, 0, 0, 1, 0, 0, 0, 31, 0, 0, 0, 32, 0, 4, 0, 40, 0, 0, 0, 1, 0, 0, 0, 30, 0, 0, 0, 21, 0, 4, 0, 41, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 42, 0, 0, 0, 1, 0, 0, 0, 41, 0, 0, 0, 32, 0, 4, 0, 43, 0, 0, 0, 1, 0, 0, 0, 34, 0, 0, 0, 43, 0, 4, 0, 41, 0, 0, 0, 44, 0, 0, 0, 1, 0, 0, 0, 20, 0, 2, 0, 45, 0, 0, 0, 28, 0, 4, 0, 27, 0, 0, 0, 31, 0, 0, 0, 35, 0, 0, 0, 30, 0, 3, 0, 20, 0, 0, 0, 27, 0, 0, 0, 30, 0, 10, 0, 21, 0, 0, 0, 20, 0, 0, 0, 20, 0, 0, 0, 20, 0, 0, 0, 20, 0, 0, 0, 20, 0, 0, 0, 20, 0, 0, 0, 33, 0, 0, 0, 33, 0, 0, 0, 30, 0, 3, 0, 22, 0, 0, 0, 21, 0, 0, 0, 32, 0, 4, 0, 46, 0, 0, 0, 2, 0, 0, 0, 22, 0, 0, 0, 43, 0, 4, 0, 34, 0, 0, 0, 47, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 34, 0, 0, 0, 48, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 49, 0, 0, 0, 2, 0, 0, 0, 20, 0, 0, 0, 43, 0, 4, 0, 30, 0, 0, 0, 50, 0, 0, 0, 0, 0, 0, 63, 32, 0, 4, 0, 51, 0, 0, 0, 7, 0, 0, 0, 33, 0, 0, 0, 43, 0, 4, 0, 30, 0, 0, 0, 52, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 30, 0, 0, 0, 53, 0, 0, 0, 0, 0, 128, 63, 32, 0, 4, 0, 54, 0, 0, 0, 3, 0, 0, 0, 31, 0, 0, 0, 32, 0, 4, 0, 55, 0, 0, 0, 3, 0, 0, 0, 33, 0, 0, 0, 59, 0, 4, 0, 38, 0, 0, 0, 10, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 38, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 38, 0, 0, 0, 12, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 39, 0, 0, 0, 13, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 39, 0, 0, 0, 14, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 40, 0, 0, 0, 15, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 42, 0, 0, 0, 16, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 43, 0, 0, 0, 18, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 43, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 3, 0, 0, 0, 2, 0, 0, 0, 59, 0, 4, 0, 54, 0, 0, 0, 4, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 54, 0, 0, 0, 5, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 54, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 55, 0, 0, 0, 7, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 55, 0, 0, 0, 8, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 55, 0, 0, 0, 9, 0, 0, 0, 3, 0, 0, 0, 44, 0, 5, 0, 33, 0, 0, 0, 56, 0, 0, 0, 50, 0, 0, 0, 50, 0, 0, 0, 54, 0, 5, 0, 28, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 29, 0, 0, 0, 248, 0, 2, 0, 57, 0, 0, 0, 59, 0, 4, 0, 37, 0, 0, 0, 58, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 59, 0, 0, 0, 10, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 60, 0, 0, 0, 11, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 61, 0, 0, 0, 12, 0, 0, 0, 61, 0, 4, 0, 31, 0, 0, 0, 62, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 31, 0, 0, 0, 63, 0, 0, 0, 14, 0, 0, 0, 61, 0, 4, 0, 30, 0, 0, 0, 64, 0, 0, 0, 15, 0, 0, 0, 61, 0, 4, 0, 41, 0, 0, 0, 65, 0, 0, 0, 16, 0, 0, 0, 61, 0, 4, 0, 34, 0, 0, 0, 66, 0, 0, 0, 18, 0, 0, 0, 61, 0, 4, 0, 34, 0, 0, 0, 67, 0, 0, 0, 17, 0, 0, 0, 130, 0, 5, 0, 34, 0, 0, 0, 68, 0, 0, 0, 67, 0, 0, 0, 66, 0, 0, 0, 124, 0, 4, 0, 41, 0, 0, 0, 69, 0, 0, 0, 68, 0, 0, 0, 199, 0, 5, 0, 41, 0, 0, 0, 70, 0, 0, 0, 65, 0, 0, 0, 44, 0, 0, 0, 170, 0, 5, 0, 45, 0, 0, 0, 19, 0, 0, 0, 70, 0, 0, 0, 44, 0, 0, 0, 247, 0, 3, 0, 71, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 19, 0, 0, 0, 72, 0, 0, 0, 73, 0, 0, 0, 248, 0, 2, 0, 73, 0, 0, 0, 66, 0, 6, 0, 49, 0, 0, 0, 74, 0, 0, 0, 3, 0, 0, 0, 47, 0, 0, 0, 48, 0, 0, 0, 61, 0, 4, 0, 20, 0, 0, 0, 75, 0, 0, 0, 74, 0, 0, 0, 81, 0, 5, 0, 27, 0, 0, 0, 76, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 77, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 78, 0, 0, 0, 77, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 79, 0, 0, 0, 77, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 80, 0, 0, 0, 77, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 81, 0, 0, 0, 77, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 82, 0, 0, 0, 76, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 83, 0, 0, 0, 82, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 84, 0, 0, 0, 82, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 85, 0, 0, 0, 82, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 86, 0, 0, 0, 82, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 87, 0, 0, 0, 76, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 88, 0, 0, 0, 87, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 89, 0, 0, 0, 87, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 90, 0, 0, 0, 87, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 91, 0, 0, 0, 87, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 92, 0, 0, 0, 76, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 93, 0, 0, 0, 92, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 94, 0, 0, 0, 92, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 95, 0, 0, 0, 92, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 96, 0, 0, 0, 92, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 97, 0, 0, 0, 78, 0, 0, 0, 83, 0, 0, 0, 88, 0, 0, 0, 93, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 98, 0, 0, 0, 79, 0, 0, 0, 84, 0, 0, 0, 89, 0, 0, 0, 94, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 99, 0, 0, 0, 80, 0, 0, 0, 85, 0, 0, 0, 90, 0, 0, 0, 95, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 100, 0, 0, 0, 81, 0, 0, 0, 86, 0, 0, 0, 91, 0, 0, 0, 96, 0, 0, 0, 80, 0, 7, 0, 32, 0, 0, 0, 101, 0, 0, 0, 97, 0, 0, 0, 98, 0, 0, 0, 99, 0, 0, 0, 100, 0, 0, 0, 249, 0, 2, 0, 71, 0, 0, 0, 248, 0, 2, 0, 72, 0, 0, 0, 66, 0, 6, 0, 49, 0, 0, 0, 102, 0, 0, 0, 3, 0, 0, 0, 47, 0, 0, 0, 47, 0, 0, 0, 61, 0, 4, 0, 20, 0, 0, 0, 103, 0, 0, 0, 102, 0, 0, 0, 81, 0, 5, 0, 27, 0, 0, 0, 104, 0, 0, 0, 103, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 105, 0, 0, 0, 104, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 106, 0, 0, 0, 105, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 107, 0, 0, 0, 105, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 108, 0, 0, 0, 105, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 109, 0, 0, 0, 105, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 110, 0, 0, 0, 104, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 111, 0, 0, 0, 110, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 112, 0, 0, 0, 110, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 113, 0, 0, 0, 110, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 114, 0, 0, 0, 110, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 115, 0, 0, 0, 104, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 116, 0, 0, 0, 115, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 117, 0, 0, 0, 115, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 118, 0, 0, 0, 115, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 119, 0, 0, 0, 115, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 120, 0, 0, 0, 104, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 121, 0, 0, 0, 120, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 122, 0, 0, 0, 120, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 123, 0, 0, 0, 120, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 124, 0, 0, 0, 120, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 125, 0, 0, 0, 106, 0, 0, 0, 111, 0, 0, 0, 116, 0, 0, 0, 121, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 126, 0, 0, 0, 107, 0, 0, 0, 112, 0, 0, 0, 117, 0, 0, 0, 122, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 127, 0, 0, 0, 108, 0, 0, 0, 113, 0, 0, 0, 118, 0, 0, 0, 123, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 128, 0, 0, 0, 109, 0, 0, 0, 114, 0, 0, 0, 119, 0, 0, 0, 124, 0, 0, 0, 80, 0, 7, 0, 32, 0, 0, 0, 129, 0, 0, 0, 125, 0, 0, 0, 126, 0, 0, 0, 127, 0, 0, 0, 128, 0, 0, 0, 249, 0, 2, 0, 71, 0, 0, 0, 248, 0, 2, 0, 71, 0, 0, 0, 245, 0, 7, 0, 32, 0, 0, 0, 130, 0, 0, 0, 101, 0, 0, 0, 73, 0, 0, 0, 129, 0, 0, 0, 72, 0, 0, 0, 131, 0, 5, 0, 33, 0, 0, 0, 23, 0, 0, 0, 61, 0, 0, 0, 60, 0, 0, 0, 12, 0, 6, 0, 30, 0, 0, 0, 131, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 23, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 132, 0, 0, 0, 23, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 133, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 127, 0, 4, 0, 30, 0, 0, 0, 134, 0, 0, 0, 133, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 135, 0, 0, 0, 132, 0, 0, 0, 134, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 136, 0, 0, 0, 131, 0, 0, 0, 131, 0, 0, 0, 136, 0, 5, 0, 33, 0, 0, 0, 137, 0, 0, 0, 135, 0, 0, 0, 136, 0, 0, 0, 142, 0, 5, 0, 33, 0, 0, 0, 138, 0, 0, 0, 137, 0, 0, 0, 64, 0, 0, 0, 142, 0, 5, 0, 33, 0, 0, 0, 24, 0, 0, 0, 138, 0, 0, 0, 50, 0, 0, 0, 131, 0, 5, 0, 33, 0, 0, 0, 139, 0, 0, 0, 60, 0, 0, 0, 24, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 140, 0, 0, 0, 60, 0, 0, 0, 24, 0, 0, 0, 131, 0, 5, 0, 33, 0, 0, 0, 141, 0, 0, 0, 61, 0, 0, 0, 24, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 142, 0, 0, 0, 61, 0, 0, 0, 24, 0, 0, 0, 80, 0, 7, 0, 36, 0, 0, 0, 25, 0, 0, 0, 139, 0, 0, 0, 140, 0, 0, 0, 141, 0, 0, 0, 142, 0, 0, 0, 62, 0, 3, 0, 58, 0, 0, 0, 25, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 26, 0, 0, 0, 131, 0, 0, 0, 64, 0, 0, 0, 131, 0, 5, 0, 33, 0, 0, 0, 143, 0, 0, 0, 59, 0, 0, 0, 56, 0, 0, 0, 133, 0, 5, 0, 33, 0, 0, 0, 144, 0, 0, 0, 143, 0, 0, 0, 26, 0, 0, 0, 65, 0, 5, 0, 51, 0, 0, 0, 145, 0, 0, 0, 58, 0, 0, 0, 69, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 146, 0, 0, 0, 145, 0, 0, 0, 80, 0, 6, 0, 31, 0, 0, 0, 147, 0, 0, 0, 146, 0, 0, 0, 52, 0, 0, 0, 53, 0, 0, 0, 144, 0, 5, 0, 31, 0, 0, 0, 148, 0, 0, 0, 147, 0, 0, 0, 130, 0, 0, 0, 82, 0, 6, 0, 31, 0, 0, 0, 149, 0, 0, 0, 53, 0, 0, 0, 148, 0, 0, 0, 2, 0, 0, 0, 62, 0, 3, 0, 4, 0, 0, 0, 149, 0, 0, 0, 62, 0, 3, 0, 5, 0, 0, 0, 62, 0, 0, 0, 62, 0, 3, 0, 6, 0, 0, 0, 63, 0, 0, 0, 62, 0, 3, 0, 7, 0, 0, 0, 26, 0, 0, 0, 62, 0, 3, 0, 8, 0, 0, 0, 144, 0, 0, 0, 62, 0, 3, 0, 9, 0, 0, 0, 59, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_LINE_FRAG[2036] = {3, 2, 35, 7, 0, 5, 1, 0, 0, 0, 40, 0, 83, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 10, 0, 4, 0, 0, 0, 2, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 16, 0, 3, 0, 2, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 11, 0, 0, 0, 1, 0, 0, 0, 5, 0, 5, 0, 4, 0, 0, 0, 105, 110, 112, 46, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 7, 0, 5, 0, 0, 0, 105, 110, 112, 46, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 0, 0, 5, 0, 5, 0, 6, 0, 0, 0, 105, 110, 112, 46, 115, 105, 122, 101, 0, 0, 0, 0, 5, 0, 4, 0, 7, 0, 0, 0, 105, 110, 112, 46, 112, 0, 0, 0, 5, 0, 7, 0, 3, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 80, 83, 0, 0, 5, 0, 3, 0, 2, 0, 0, 0, 80, 83, 0, 0, 71, 0, 4, 0, 4, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 4, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 5, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 3, 0, 5, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 3, 0, 6, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 7, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 8, 0, 0, 0, 33, 0, 3, 0, 9, 0, 0, 0, 8, 0, 0, 0, 22, 0, 3, 0, 10, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 11, 0, 0, 0, 10, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 12, 0, 0, 0, 1, 0, 0, 0, 11, 0, 0, 0, 23, 0, 4, 0, 13, 0, 0, 0, 10, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 14, 0, 0, 0, 1, 0, 0, 0, 13, 0, 0, 0, 20, 0, 2, 0, 15, 0, 0, 0, 43, 0, 4, 0, 10, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 10, 0, 0, 0, 17, 0, 0, 0, 0, 0, 0, 63, 44, 0, 5, 0, 13, 0, 0, 0, 18, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 43, 0, 4, 0, 10, 0, 0, 0, 19, 0, 0, 0, 46, 186, 232, 62, 43, 0, 4, 0, 10, 0, 0, 0, 20, 0, 0, 0, 0, 0, 128, 63, 23, 0, 4, 0, 21, 0, 0, 0, 10, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 10, 0, 0, 0, 22, 0, 0, 0, 205, 204, 76, 61, 32, 0, 4, 0, 23, 0, 0, 0, 3, 0, 0, 0, 11, 0, 0, 0, 59, 0, 4, 0, 12, 0, 0, 0, 4, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 12, 0, 0, 0, 5, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 14, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 14, 0, 0, 0, 7, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 23, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 8, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 248, 0, 2, 0, 24, 0, 0, 0, 61, 0, 4, 0, 11, 0, 0, 0, 25, 0, 0, 0, 4, 0, 0, 0, 61, 0, 4, 0, 11, 0, 0, 0, 26, 0, 0, 0, 5, 0, 0, 0, 61, 0, 4, 0, 13, 0, 0, 0, 27, 0, 0, 0, 6, 0, 0, 0, 61, 0, 4, 0, 13, 0, 0, 0, 28, 0, 0, 0, 7, 0, 0, 0, 81, 0, 5, 0, 10, 0, 0, 0, 29, 0, 0, 0, 26, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 10, 0, 0, 0, 30, 0, 0, 0, 26, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 10, 0, 0, 0, 31, 0, 0, 0, 26, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 10, 0, 0, 0, 32, 0, 0, 0, 26, 0, 0, 0, 3, 0, 0, 0, 12, 0, 7, 0, 10, 0, 0, 0, 33, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 31, 0, 0, 0, 32, 0, 0, 0, 12, 0, 7, 0, 10, 0, 0, 0, 34, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 30, 0, 0, 0, 33, 0, 0, 0, 12, 0, 7, 0, 10, 0, 0, 0, 35, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 29, 0, 0, 0, 34, 0, 0, 0, 186, 0, 5, 0, 15, 0, 0, 0, 36, 0, 0, 0, 35, 0, 0, 0, 16, 0, 0, 0, 247, 0, 3, 0, 37, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 36, 0, 0, 0, 38, 0, 0, 0, 39, 0, 0, 0, 248, 0, 2, 0, 39, 0, 0, 0, 249, 0, 2, 0, 37, 0, 0, 0, 248, 0, 2, 0, 38, 0, 0, 0, 81, 0, 5, 0, 10, 0, 0, 0, 40, 0, 0, 0, 28, 0, 0, 0, 1, 0, 0, 0, 184, 0, 5, 0, 15, 0, 0, 0, 41, 0, 0, 0, 16, 0, 0, 0, 40, 0, 0, 0, 247, 0, 3, 0, 42, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 41, 0, 0, 0, 43, 0, 0, 0, 44, 0, 0, 0, 248, 0, 2, 0, 44, 0, 0, 0, 79, 0, 7, 0, 13, 0, 0, 0, 45, 0, 0, 0, 26, 0, 0, 0, 26, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 249, 0, 2, 0, 42, 0, 0, 0, 248, 0, 2, 0, 43, 0, 0, 0, 79, 0, 7, 0, 13, 0, 0, 0, 46, 0, 0, 0, 26, 0, 0, 0, 26, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 249, 0, 2, 0, 42, 0, 0, 0, 248, 0, 2, 0, 42, 0, 0, 0, 245, 0, 7, 0, 13, 0, 0, 0, 47, 0, 0, 0, 45, 0, 0, 0, 44, 0, 0, 0, 46, 0, 0, 0, 43, 0, 0, 0, 81, 0, 5, 0, 10, 0, 0, 0, 48, 0, 0, 0, 28, 0, 0, 0, 0, 0, 0, 0, 184, 0, 5, 0, 15, 0, 0, 0, 49, 0, 0, 0, 16, 0, 0, 0, 48, 0, 0, 0, 247, 0, 3, 0, 50, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 49, 0, 0, 0, 51, 0, 0, 0, 52, 0, 0, 0, 248, 0, 2, 0, 52, 0, 0, 0, 81, 0, 5, 0, 10, 0, 0, 0, 53, 0, 0, 0, 47, 0, 0, 0, 0, 0, 0, 0, 249, 0, 2, 0, 50, 0, 0, 0, 248, 0, 2, 0, 51, 0, 0, 0, 81, 0, 5, 0, 10, 0, 0, 0, 54, 0, 0, 0, 47, 0, 0, 0, 1, 0, 0, 0, 249, 0, 2, 0, 50, 0, 0, 0, 248, 0, 2, 0, 50, 0, 0, 0, 245, 0, 7, 0, 10, 0, 0, 0, 55, 0, 0, 0, 53, 0, 0, 0, 52, 0, 0, 0, 54, 0, 0, 0, 51, 0, 0, 0, 12, 0, 6, 0, 13, 0, 0, 0, 56, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 28, 0, 0, 0, 142, 0, 5, 0, 13, 0, 0, 0, 57, 0, 0, 0, 27, 0, 0, 0, 17, 0, 0, 0, 131, 0, 5, 0, 13, 0, 0, 0, 58, 0, 0, 0, 56, 0, 0, 0, 57, 0, 0, 0, 80, 0, 5, 0, 13, 0, 0, 0, 59, 0, 0, 0, 55, 0, 0, 0, 55, 0, 0, 0, 129, 0, 5, 0, 13, 0, 0, 0, 60, 0, 0, 0, 58, 0, 0, 0, 59, 0, 0, 0, 12, 0, 7, 0, 13, 0, 0, 0, 61, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 60, 0, 0, 0, 18, 0, 0, 0, 12, 0, 6, 0, 10, 0, 0, 0, 62, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 61, 0, 0, 0, 81, 0, 5, 0, 10, 0, 0, 0, 63, 0, 0, 0, 60, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 10, 0, 0, 0, 64, 0, 0, 0, 60, 0, 0, 0, 1, 0, 0, 0, 12, 0, 7, 0, 10, 0, 0, 0, 65, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 63, 0, 0, 0, 64, 0, 0, 0, 12, 0, 7, 0, 10, 0, 0, 0, 66, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 65, 0, 0, 0, 16, 0, 0, 0, 129, 0, 5, 0, 10, 0, 0, 0, 67, 0, 0, 0, 62, 0, 0, 0, 66, 0, 0, 0, 131, 0, 5, 0, 10, 0, 0, 0, 68, 0, 0, 0, 67, 0, 0, 0, 55, 0, 0, 0, 209, 0, 4, 0, 10, 0, 0, 0, 69, 0, 0, 0, 68, 0, 0, 0, 12, 0, 7, 0, 10, 0, 0, 0, 70, 0, 0, 0, 1, 0, 0, 0, 26, 0, 0, 0, 69, 0, 0, 0, 19, 0, 0, 0, 127, 0, 4, 0, 10, 0, 0, 0, 71, 0, 0, 0, 70, 0, 0, 0, 12, 0, 8, 0, 10, 0, 0, 0, 72, 0, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 71, 0, 0, 0, 16, 0, 0, 0, 68, 0, 0, 0, 131, 0, 5, 0, 10, 0, 0, 0, 73, 0, 0, 0, 20, 0, 0, 0, 72, 0, 0, 0, 79, 0, 8, 0, 21, 0, 0, 0, 74, 0, 0, 0, 25, 0, 0, 0, 25, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 10, 0, 0, 0, 75, 0, 0, 0, 25, 0, 0, 0, 3, 0, 0, 0, 12, 0, 7, 0, 10, 0, 0, 0, 76, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 75, 0, 0, 0, 73, 0, 0, 0, 80, 0, 5, 0, 11, 0, 0, 0, 77, 0, 0, 0, 74, 0, 0, 0, 76, 0, 0, 0, 249, 0, 2, 0, 37, 0, 0, 0, 248, 0, 2, 0, 37, 0, 0, 0, 245, 0, 7, 0, 11, 0, 0, 0, 78, 0, 0, 0, 25, 0, 0, 0, 39, 0, 0, 0, 77, 0, 0, 0, 50, 0, 0, 0, 81, 0, 5, 0, 10, 0, 0, 0, 79, 0, 0, 0, 78, 0, 0, 0, 3, 0, 0, 0, 188, 0, 5, 0, 15, 0, 0, 0, 80, 0, 0, 0, 79, 0, 0, 0, 22, 0, 0, 0, 247, 0, 3, 0, 81, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 80, 0, 0, 0, 82, 0, 0, 0, 81, 0, 0, 0, 248, 0, 2, 0, 82, 0, 0, 0, 252, 0, 1, 0, 248, 0, 2, 0, 81, 0, 0, 0, 62, 0, 3, 0, 3, 0, 0, 0, 78, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const char METAL_LINE_VERT[6135] = R"(#include <metal_stdlib>
#include <metal_math>
#include <metal_texture>
using namespace metal;
struct VS_Result_0
{
float4 position_0 [[position]];
float4 color_0 [[user(COLOR)]];
float4 border_radius_0 [[user(BORDERRADIUS)]];
float2 size_0 [[user(SIZE)]];
float2 p_0 [[user(POINT)]];
float2 uv_0 [[user(UV)]];
};
struct vertexInput_0
{
float2 position_1 [[attribute(0)]];
float2 i_start_0 [[attribute(1)]];
float2 i_end_0 [[attribute(2)]];
float4 i_color_0 [[attribute(3)]];
float4 i_border_radius_0 [[attribute(4)]];
float i_thickness_0 [[attribute(5)]];
uint i_flags_0 [[attribute(6)]];
};
struct _MatrixStorage_float4x4_ColMajornatural_0
{
array<float4, int(4)> data_0;
};
struct GlobalUniforms_natural_0
{
_MatrixStorage_float4x4_ColMajornatural_0 screen_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 view_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 nonscale_view_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 nonscale_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 transform_matrix_0;
_MatrixStorage_float4x4_ColMajornatural_0 inv_view_proj_0;
float2 camera_position_0;
float2 window_size_0;
};
struct SLANG_ParameterGroup_GlobalUniformBuffer_natural_0
{
GlobalUniforms_natural_0 uniforms_0;
};
struct KernelContext_0
{
SLANG_ParameterGroup_GlobalUniformBuffer_natural_0 constant* GlobalUniformBuffer_0;
};
struct VSOutput_0
{
float4 position_2;
[[flat]] float4 color_1;
[[flat]] float4 border_radius_1;
[[flat]] float2 size_1;
float2 p_1;
float2 uv_1;
};
[[vertex]] VS_Result_0 VS(vertexInput_0 _S1 [[stage_in]], uint vid_0 [[vertex_id]], SLANG_ParameterGroup_GlobalUniformBuffer_natural_0 constant* GlobalUniformBuffer_1 [[buffer(2)]])
{
KernelContext_0 kernelContext_0;
(&kernelContext_0)->GlobalUniformBuffer_0 = GlobalUniformBuffer_1;
matrix<float,int(4),int(4)>  mvp_0;
if(((_S1.i_flags_0) & 1U) == 1U)
{
mvp_0 = matrix<float,int(4),int(4)> ((&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(3)]);
}
else
{
mvp_0 = matrix<float,int(4),int(4)> ((&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(3)]);
}
float2 d_0 = _S1.i_end_0 - _S1.i_start_0;
float len_0 = length(d_0);
float2 _S2 = float2(0.5) ;
float2 perp_0 = float2(d_0.y, - d_0.x) / float2(len_0)  * float2(_S1.i_thickness_0)  * _S2;
array<float2, int(4)> vertices_0 = { _S1.i_start_0 - perp_0, _S1.i_start_0 + perp_0, _S1.i_end_0 - perp_0, _S1.i_end_0 + perp_0 };
float2 size_2 = float2(len_0, _S1.i_thickness_0);
thread VSOutput_0 outp_0;
(&outp_0)->color_1 = _S1.i_color_0;
(&outp_0)->border_radius_1 = _S1.i_border_radius_0;
(&outp_0)->size_1 = size_2;
(&outp_0)->p_1 = (_S1.position_1 - _S2) * size_2;
(&outp_0)->uv_1 = _S1.position_1;
(&outp_0)->position_2 = (((float4(vertices_0[vid_0], 0.0, 1.0)) * (mvp_0)));
(&outp_0)->position_2.z = 1.0;
VSOutput_0 _S3 = outp_0;
thread VS_Result_0 _S4;
(&_S4)->position_0 = _S3.position_2;
(&_S4)->color_0 = _S3.color_1;
(&_S4)->border_radius_0 = _S3.border_radius_1;
(&_S4)->size_0 = _S3.size_1;
(&_S4)->p_0 = _S3.p_1;
(&_S4)->uv_0 = _S3.uv_1;
return _S4;
}
)";

static const char METAL_LINE_FRAG[1526] = R"(#include <metal_stdlib>
#include <metal_math>
#include <metal_texture>
using namespace metal;
float sd_rounded_box_0(const float2 thread* p_0, const float2 thread* size_0, const float4 thread* corner_radii_0)
{
float2 rs_0;
if(0.0 < ((*p_0).y))
{
rs_0 = (*corner_radii_0).zw;
}
else
{
rs_0 = (*corner_radii_0).xy;
}
float radius_0;
if(0.0 < ((*p_0).x))
{
radius_0 = rs_0.y;
}
else
{
radius_0 = rs_0.x;
}
float2 q_0 = abs(*p_0) - float2(0.5)  * *size_0 + float2(radius_0) ;
return length(max(q_0, float2(0.0, 0.0))) + min(max(q_0.x, q_0.y), 0.0) - radius_0;
}
float antialias_0(float d_0)
{
return 1.0 - smoothstep(- pow((fwidth((d_0))), 0.45454543828964233), 0.0, d_0);
}
struct pixelOutput_0
{
float4 output_0 [[color(0)]];
};
struct pixelInput_0
{
[[flat]] float4 color_0 [[user(COLOR)]];
[[flat]] float4 border_radius_0 [[user(BORDERRADIUS)]];
[[flat]] float2 size_1 [[user(SIZE)]];
float2 p_1 [[user(POINT)]];
float2 uv_0 [[user(UV)]];
};
[[fragment]] pixelOutput_0 PS(pixelInput_0 _S1 [[stage_in]], float4 position_0 [[position]])
{
float4 result_0;
if((max(_S1.border_radius_0.x, max(_S1.border_radius_0.y, max(_S1.border_radius_0.z, _S1.border_radius_0.w)))) > 0.0)
{
float2 _S2 = _S1.p_1;
float2 _S3 = _S1.size_1;
float4 _S4 = _S1.border_radius_0;
float _S5 = sd_rounded_box_0(&_S2, &_S3, &_S4);
result_0 = float4(_S1.color_0.xyz, min(_S1.color_0.w, antialias_0(_S5)));
}
else
{
result_0 = _S1.color_0;
}
if((result_0.w) <= 0.05000000074505806)
{
discard_fragment();
}
pixelOutput_0 _S6 = { result_0 };
return _S6;
}
)";

static const char GL_LINE_VERT[4016] = R"(#version 410
#ifdef GL_ARB_shader_draw_parameters
#extension GL_ARB_shader_draw_parameters : enable
#endif
struct _MatrixStorage_float4x4_ColMajorstd140
{
vec4 data[4];
};
struct GlobalUniforms_std140
{
_MatrixStorage_float4x4_ColMajorstd140 screen_projection;
_MatrixStorage_float4x4_ColMajorstd140 view_projection;
_MatrixStorage_float4x4_ColMajorstd140 nonscale_view_projection;
_MatrixStorage_float4x4_ColMajorstd140 nonscale_projection;
_MatrixStorage_float4x4_ColMajorstd140 transform_matrix;
_MatrixStorage_float4x4_ColMajorstd140 inv_view_proj;
vec2 camera_position;
vec2 window_size;
};
layout(std140) uniform GlobalUniformBuffer_std140
{
GlobalUniforms_std140 uniforms;
} GlobalUniformBuffer;
layout(location = 0) in vec2 inp_position;
layout(location = 1) in vec2 inp_i_start;
layout(location = 2) in vec2 inp_i_end;
layout(location = 3) in vec4 inp_i_color;
layout(location = 4) in vec4 inp_i_border_radius;
layout(location = 5) in float inp_i_thickness;
layout(location = 6) in uint inp_i_flags;
#ifdef GL_ARB_shader_draw_parameters
#define SPIRV_Cross_BaseVertex gl_BaseVertexARB
#else
uniform int SPIRV_Cross_BaseVertex;
#endif
layout(location = 0) flat out vec4 entryPointParam_VS_color;
layout(location = 1) flat out vec4 entryPointParam_VS_border_radius;
layout(location = 2) flat out vec2 entryPointParam_VS_size;
layout(location = 3) out vec2 entryPointParam_VS_p;
layout(location = 4) out vec2 entryPointParam_VS_uv;
void main()
{
mat4 _130;
if ((inp_i_flags & 1u) == 1u)
{
_130 = mat4(vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].x, GlobalUniformBuffer.uniforms.screen_projection.data[1].x, GlobalUniformBuffer.uniforms.screen_projection.data[2].x, GlobalUniformBuffer.uniforms.screen_projection.data[3].x), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].y, GlobalUniformBuffer.uniforms.screen_projection.data[1].y, GlobalUniformBuffer.uniforms.screen_projection.data[2].y, GlobalUniformBuffer.uniforms.screen_projection.data[3].y), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].z, GlobalUniformBuffer.uniforms.screen_projection.data[1].z, GlobalUniformBuffer.uniforms.screen_projection.data[2].z, GlobalUniformBuffer.uniforms.screen_projection.data[3].z), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].w, GlobalUniformBuffer.uniforms.screen_projection.data[1].w, GlobalUniformBuffer.uniforms.screen_projection.data[2].w, GlobalUniformBuffer.uniforms.screen_projection.data[3].w));
}
else
{
_130 = mat4(vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].x, GlobalUniformBuffer.uniforms.view_projection.data[1].x, GlobalUniformBuffer.uniforms.view_projection.data[2].x, GlobalUniformBuffer.uniforms.view_projection.data[3].x), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].y, GlobalUniformBuffer.uniforms.view_projection.data[1].y, GlobalUniformBuffer.uniforms.view_projection.data[2].y, GlobalUniformBuffer.uniforms.view_projection.data[3].y), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].z, GlobalUniformBuffer.uniforms.view_projection.data[1].z, GlobalUniformBuffer.uniforms.view_projection.data[2].z, GlobalUniformBuffer.uniforms.view_projection.data[3].z), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].w, GlobalUniformBuffer.uniforms.view_projection.data[1].w, GlobalUniformBuffer.uniforms.view_projection.data[2].w, GlobalUniformBuffer.uniforms.view_projection.data[3].w));
}
vec2 d = inp_i_end - inp_i_start;
float _131 = length(d);
vec2 perp = ((vec2(d.y, -d.x) / vec2(_131)) * inp_i_thickness) * 0.5;
vec2 _58[4] = vec2[](inp_i_start - perp, inp_i_start + perp, inp_i_end - perp, inp_i_end + perp);
vec2 size = vec2(_131, inp_i_thickness);
vec4 _148 = vec4(_58[uint(gl_VertexID - SPIRV_Cross_BaseVertex)], 0.0, 1.0) * _130;
_148.z = 1.0;
gl_Position = _148;
entryPointParam_VS_color = inp_i_color;
entryPointParam_VS_border_radius = inp_i_border_radius;
entryPointParam_VS_size = size;
entryPointParam_VS_p = (inp_position - vec2(0.5)) * size;
entryPointParam_VS_uv = inp_position;
}
)";

static const char GL_LINE_FRAG[913] = R"(#version 410
layout(location = 0) flat in vec4 inp_color;
layout(location = 1) flat in vec4 inp_border_radius;
layout(location = 2) flat in vec2 inp_size;
layout(location = 3) in vec2 inp_p;
layout(location = 0) out vec4 entryPointParam_PS;
void main()
{
vec4 _78;
if (max(inp_border_radius.x, max(inp_border_radius.y, max(inp_border_radius.z, inp_border_radius.w))) > 0.0)
{
vec2 _47;
if (0.0 < inp_p.y)
{
_47 = inp_border_radius.zw;
}
else
{
_47 = inp_border_radius.xy;
}
float _55;
if (0.0 < inp_p.x)
{
_55 = _47.y;
}
else
{
_55 = _47.x;
}
vec2 _60 = (abs(inp_p) - (inp_size * 0.5)) + vec2(_55);
float _68 = (length(max(_60, vec2(0.0))) + min(max(_60.x, _60.y), 0.0)) - _55;
_78 = vec4(inp_color.xyz, min(inp_color.w, 1.0 - smoothstep(-pow(fwidth(_68), 0.454545438289642333984375), 0.0, _68)));
}
else
{
_78 = inp_color;
}
if (_78.w <= 0.0500000007450580596923828125)
{
discard;
}
entryPointParam_PS = _78;
}
)";

static const char D3D11_NINEPATCH_VERT[4443] = R"(#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#ifndef __DXC_VERSION_MAJOR
#pragma warning(disable : 3557)
#endif
struct GlobalUniforms_0
{
float4x4 screen_projection_0;
float4x4 view_projection_0;
float4x4 nonscale_view_projection_0;
float4x4 nonscale_projection_0;
float4x4 transform_matrix_0;
float4x4 inv_view_proj_0;
float2 camera_position_0;
float2 window_size_0;
};
struct SLANG_ParameterGroup_GlobalUniformBuffer_0
{
GlobalUniforms_0 uniforms_0;
};
cbuffer GlobalUniformBuffer_0 : register(b2)
{
SLANG_ParameterGroup_GlobalUniformBuffer_0 GlobalUniformBuffer_0;
}
struct VSOutput_0
{
float4 position_0 : SV_Position;
nointerpolation float4 color_0 : Color;
nointerpolation uint4 margin_0 : Margin;
nointerpolation float2 source_size_0 : SourceSize;
nointerpolation float2 output_size_0 : OutputSize;
float2 uv_0 : UV;
};
struct VSInput_0
{
float2 position_1 : Position;
float2 i_position_0 : I_Position;
float4 i_rotation_0 : I_Rotation;
float2 i_size_0 : I_Size;
float2 i_offset_0 : I_Offset;
float2 i_source_size_0 : I_SourceSize;
float2 i_output_size_0 : I_OutputSize;
uint4 i_margin_0 : I_Margin;
float4 i_uv_offset_scale_0 : I_UvOffsetScale;
float4 i_color_0 : I_Color;
uint i_flags_0 : I_Flags;
};
VSOutput_0 VS(VSInput_0 inp_0)
{
VSInput_0 _S1 = inp_0;
float _S2 = inp_0.i_rotation_0.x;
float qxx_0 = _S2 * _S2;
float _S3 = inp_0.i_rotation_0.y;
float qyy_0 = _S3 * _S3;
float _S4 = inp_0.i_rotation_0.z;
float qzz_0 = _S4 * _S4;
float qxz_0 = _S2 * _S4;
float qxy_0 = _S2 * _S3;
float qyz_0 = _S3 * _S4;
float _S5 = inp_0.i_rotation_0.w;
float qwx_0 = _S5 * _S2;
float qwy_0 = _S5 * _S3;
float qwz_0 = _S5 * _S4;
float4 _S6 = float4(0.0f, 0.0f, 0.0f, 1.0f);
float4x4 transform_0 = mul(float4x4(float4(1.0f, 0.0f, 0.0f, inp_0.i_position_0.x), float4(0.0f, 1.0f, 0.0f, inp_0.i_position_0.y), float4(0.0f, 0.0f, 1.0f, 0.0f), _S6), float4x4(float4(1.0f - 2.0f * (qyy_0 + qzz_0), 2.0f * (qxy_0 - qwz_0), 2.0f * (qxz_0 + qwy_0), 0.0f), float4(2.0f * (qxy_0 + qwz_0), 1.0f - 2.0f * (qxx_0 + qzz_0), 2.0f * (qyz_0 - qwx_0), 0.0f), float4(2.0f * (qxz_0 - qwy_0), 2.0f * (qyz_0 + qwx_0), 1.0f - 2.0f * (qxx_0 + qyy_0), 0.0f), _S6));
float2 offset_0 = - inp_0.i_offset_0 * inp_0.i_size_0;
transform_0[int(0)][int(3)] = transform_0[int(0)][int(0)] * offset_0[int(0)] + transform_0[int(0)][int(1)] * offset_0[int(1)] + transform_0[int(0)][int(2)] * 0.0f + transform_0[int(0)][int(3)];
transform_0[int(1)][int(3)] = transform_0[int(1)][int(0)] * offset_0[int(0)] + transform_0[int(1)][int(1)] * offset_0[int(1)] + transform_0[int(1)][int(2)] * 0.0f + transform_0[int(1)][int(3)];
transform_0[int(2)][int(3)] = transform_0[int(2)][int(0)] * offset_0[int(0)] + transform_0[int(2)][int(1)] * offset_0[int(1)] + transform_0[int(2)][int(2)] * 0.0f + transform_0[int(2)][int(3)];
transform_0[int(3)][int(3)] = transform_0[int(3)][int(0)] * offset_0[int(0)] + transform_0[int(3)][int(1)] * offset_0[int(1)] + transform_0[int(3)][int(2)] * 0.0f + transform_0[int(3)][int(3)];
transform_0[int(0)][int(0)] = transform_0[int(0)][int(0)] * inp_0.i_size_0[int(0)];
transform_0[int(1)][int(0)] = transform_0[int(1)][int(0)] * inp_0.i_size_0[int(0)];
transform_0[int(2)][int(0)] = transform_0[int(2)][int(0)] * inp_0.i_size_0[int(0)];
transform_0[int(3)][int(0)] = transform_0[int(3)][int(0)] * inp_0.i_size_0[int(0)];
transform_0[int(0)][int(1)] = transform_0[int(0)][int(1)] * inp_0.i_size_0[int(1)];
transform_0[int(1)][int(1)] = transform_0[int(1)][int(1)] * inp_0.i_size_0[int(1)];
transform_0[int(2)][int(1)] = transform_0[int(2)][int(1)] * inp_0.i_size_0[int(1)];
transform_0[int(3)][int(1)] = transform_0[int(3)][int(1)] * inp_0.i_size_0[int(1)];
bool ignore_camera_zoom_0 = (uint(int(inp_0.i_flags_0)) & 2U) == 2U;
float4x4 _S7;
if(((inp_0.i_flags_0) & 1U) == 1U)
{
_S7 = GlobalUniformBuffer_0.uniforms_0.screen_projection_0;
}
else
{
if(ignore_camera_zoom_0)
{
_S7 = GlobalUniformBuffer_0.uniforms_0.nonscale_view_projection_0;
}
else
{
_S7 = GlobalUniformBuffer_0.uniforms_0.view_projection_0;
}
}
VSOutput_0 outp_0;
outp_0.position_0 = mul(mul(_S7, transform_0), float4(_S1.position_1, 0.0f, 1.0f));
outp_0.position_0[int(2)] = 1.0f;
outp_0.uv_0 = _S1.position_1 * _S1.i_uv_offset_scale_0.zw + _S1.i_uv_offset_scale_0.xy;
outp_0.color_0 = _S1.i_color_0;
outp_0.margin_0 = _S1.i_margin_0;
outp_0.source_size_0 = _S1.i_source_size_0;
outp_0.output_size_0 = _S1.i_output_size_0;
return outp_0;
}
)";

static const char D3D11_NINEPATCH_FRAG[1516] = R"(#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#ifndef __DXC_VERSION_MAJOR
#pragma warning(disable : 3557)
#endif
Texture2D<float4 > Texture_0 : register(t3);
SamplerState Sampler_0 : register(s4);
float map_0(float value_0, float in_min_0, float in_max_0, float out_min_0, float out_max_0)
{
return (value_0 - in_min_0) / (in_max_0 - in_min_0) * (out_max_0 - out_min_0) + out_min_0;
}
float process_axis_0(float coord_0, float2 source_margin_0, float2 out_margin_0)
{
float _S1 = out_margin_0.x;
if(coord_0 < _S1)
{
return map_0(coord_0, 0.0f, _S1, 0.0f, source_margin_0.x);
}
float _S2 = 1.0f - out_margin_0.y;
if(coord_0 < _S2)
{
return map_0(coord_0, _S1, _S2, source_margin_0.x, 1.0f - source_margin_0.y);
}
return map_0(coord_0, _S2, 1.0f, 1.0f - source_margin_0.y, 1.0f);
}
struct VSOutput_0
{
float4 position_0 : SV_Position;
nointerpolation float4 color_0 : Color;
nointerpolation uint4 margin_0 : Margin;
nointerpolation float2 source_size_0 : SourceSize;
nointerpolation float2 output_size_0 : OutputSize;
float2 uv_0 : UV;
};
float4 PS(VSOutput_0 inp_0) : SV_TARGET
{
float2 _S3 = float2(inp_0.margin_0.xy);
float2 _S4 = float2(inp_0.margin_0.zw);
float4 color_1 = Texture_0.Sample(Sampler_0, float2(process_axis_0(inp_0.uv_0.x, _S3 / inp_0.source_size_0.xx, _S3 / inp_0.output_size_0.xx), process_axis_0(inp_0.uv_0.y, _S4 / inp_0.source_size_0.yy, _S4 / inp_0.output_size_0.yy))) * inp_0.color_0;
if((color_1.w) <= 0.5f)
{
discard;
}
return color_1;
}
)";

static const unsigned char VULKAN_NINEPATCH_VERT[7424] = {3, 2, 35, 7, 0, 5, 1, 0, 0, 0, 40, 0, 14, 1, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 23, 0, 0, 0, 0, 0, 1, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 10, 0, 0, 0, 11, 0, 0, 0, 12, 0, 0, 0, 13, 0, 0, 0, 14, 0, 0, 0, 15, 0, 0, 0, 16, 0, 0, 0, 17, 0, 0, 0, 18, 0, 0, 0, 19, 0, 0, 0, 3, 0, 3, 0, 11, 0, 0, 0, 1, 0, 0, 0, 5, 0, 6, 0, 9, 0, 0, 0, 105, 110, 112, 46, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 0, 0, 5, 0, 6, 0, 10, 0, 0, 0, 105, 110, 112, 46, 105, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 6, 0, 11, 0, 0, 0, 105, 110, 112, 46, 105, 95, 114, 111, 116, 97, 116, 105, 111, 110, 0, 0, 5, 0, 5, 0, 12, 0, 0, 0, 105, 110, 112, 46, 105, 95, 115, 105, 122, 101, 0, 0, 5, 0, 6, 0, 13, 0, 0, 0, 105, 110, 112, 46, 105, 95, 111, 102, 102, 115, 101, 116, 0, 0, 0, 0, 5, 0, 7, 0, 14, 0, 0, 0, 105, 110, 112, 46, 105, 95, 115, 111, 117, 114, 99, 101, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 7, 0, 15, 0, 0, 0, 105, 110, 112, 46, 105, 95, 111, 117, 116, 112, 117, 116, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 6, 0, 16, 0, 0, 0, 105, 110, 112, 46, 105, 95, 109, 97, 114, 103, 105, 110, 0, 0, 0, 0, 5, 0, 8, 0, 17, 0, 0, 0, 105, 110, 112, 46, 105, 95, 117, 118, 95, 111, 102, 102, 115, 101, 116, 95, 115, 99, 97, 108, 101, 0, 0, 0, 5, 0, 5, 0, 18, 0, 0, 0, 105, 110, 112, 46, 105, 95, 99, 111, 108, 111, 114, 0, 5, 0, 5, 0, 19, 0, 0, 0, 105, 110, 112, 46, 105, 95, 102, 108, 97, 103, 115, 0, 5, 0, 3, 0, 20, 0, 0, 0, 113, 120, 120, 0, 5, 0, 3, 0, 21, 0, 0, 0, 113, 121, 121, 0, 5, 0, 3, 0, 22, 0, 0, 0, 113, 122, 122, 0, 5, 0, 3, 0, 23, 0, 0, 0, 113, 120, 122, 0, 5, 0, 3, 0, 24, 0, 0, 0, 113, 120, 121, 0, 5, 0, 3, 0, 25, 0, 0, 0, 113, 121, 122, 0, 5, 0, 3, 0, 26, 0, 0, 0, 113, 119, 120, 0, 5, 0, 3, 0, 27, 0, 0, 0, 113, 119, 121, 0, 5, 0, 3, 0, 28, 0, 0, 0, 113, 119, 122, 0, 5, 0, 6, 0, 29, 0, 0, 0, 114, 111, 116, 97, 116, 105, 111, 110, 95, 109, 97, 116, 114, 105, 120, 0, 5, 0, 4, 0, 30, 0, 0, 0, 111, 102, 102, 115, 101, 116, 0, 0, 5, 0, 4, 0, 31, 0, 0, 0, 102, 108, 97, 103, 115, 0, 0, 0, 5, 0, 7, 0, 32, 0, 0, 0, 105, 103, 110, 111, 114, 101, 95, 99, 97, 109, 101, 114, 97, 95, 122, 111, 111, 109, 0, 0, 5, 0, 4, 0, 33, 0, 0, 0, 105, 115, 95, 117, 105, 0, 0, 0, 5, 0, 12, 0, 34, 0, 0, 0, 95, 77, 97, 116, 114, 105, 120, 83, 116, 111, 114, 97, 103, 101, 95, 102, 108, 111, 97, 116, 52, 120, 52, 95, 67, 111, 108, 77, 97, 106, 111, 114, 115, 116, 100, 49, 52, 48, 0, 0, 6, 0, 5, 0, 34, 0, 0, 0, 0, 0, 0, 0, 100, 97, 116, 97, 0, 0, 0, 0, 5, 0, 8, 0, 35, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 115, 95, 115, 116, 100, 49, 52, 48, 0, 0, 0, 6, 0, 8, 0, 35, 0, 0, 0, 0, 0, 0, 0, 115, 99, 114, 101, 101, 110, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 7, 0, 35, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 10, 0, 35, 0, 0, 0, 2, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 0, 6, 0, 8, 0, 35, 0, 0, 0, 3, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 8, 0, 35, 0, 0, 0, 4, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 95, 109, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 7, 0, 35, 0, 0, 0, 5, 0, 0, 0, 105, 110, 118, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 0, 0, 0, 6, 0, 7, 0, 35, 0, 0, 0, 6, 0, 0, 0, 99, 97, 109, 101, 114, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 6, 0, 35, 0, 0, 0, 7, 0, 0, 0, 119, 105, 110, 100, 111, 119, 95, 115, 105, 122, 101, 0, 5, 0, 14, 0, 36, 0, 0, 0, 83, 76, 65, 78, 71, 95, 80, 97, 114, 97, 109, 101, 116, 101, 114, 71, 114, 111, 117, 112, 95, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 95, 115, 116, 100, 49, 52, 48, 0, 6, 0, 6, 0, 36, 0, 0, 0, 0, 0, 0, 0, 117, 110, 105, 102, 111, 114, 109, 115, 0, 0, 0, 0, 5, 0, 7, 0, 2, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 5, 0, 9, 0, 4, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 99, 111, 108, 111, 114, 0, 0, 0, 0, 5, 0, 9, 0, 5, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 109, 97, 114, 103, 105, 110, 0, 0, 0, 5, 0, 10, 0, 6, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 115, 111, 117, 114, 99, 101, 95, 115, 105, 122, 101, 0, 0, 5, 0, 10, 0, 7, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 111, 117, 116, 112, 117, 116, 95, 115, 105, 122, 101, 0, 0, 5, 0, 8, 0, 8, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 117, 118, 0, 0, 0, 5, 0, 3, 0, 1, 0, 0, 0, 86, 83, 0, 0, 71, 0, 4, 0, 9, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 10, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 12, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 13, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 14, 0, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 4, 0, 15, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 4, 0, 16, 0, 0, 0, 30, 0, 0, 0, 7, 0, 0, 0, 71, 0, 4, 0, 17, 0, 0, 0, 30, 0, 0, 0, 8, 0, 0, 0, 71, 0, 4, 0, 18, 0, 0, 0, 30, 0, 0, 0, 9, 0, 0, 0, 71, 0, 4, 0, 19, 0, 0, 0, 30, 0, 0, 0, 10, 0, 0, 0, 71, 0, 4, 0, 37, 0, 0, 0, 6, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 34, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 35, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 35, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 5, 0, 35, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 72, 0, 5, 0, 35, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 192, 0, 0, 0, 72, 0, 5, 0, 35, 0, 0, 0, 4, 0, 0, 0, 35, 0, 0, 0, 0, 1, 0, 0, 72, 0, 5, 0, 35, 0, 0, 0, 5, 0, 0, 0, 35, 0, 0, 0, 64, 1, 0, 0, 72, 0, 5, 0, 35, 0, 0, 0, 6, 0, 0, 0, 35, 0, 0, 0, 128, 1, 0, 0, 72, 0, 5, 0, 35, 0, 0, 0, 7, 0, 0, 0, 35, 0, 0, 0, 136, 1, 0, 0, 71, 0, 3, 0, 36, 0, 0, 0, 2, 0, 0, 0, 72, 0, 5, 0, 36, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 2, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 2, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 4, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 4, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 5, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 3, 0, 5, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 3, 0, 6, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 7, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 3, 0, 7, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 8, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 19, 0, 2, 0, 38, 0, 0, 0, 33, 0, 3, 0, 39, 0, 0, 0, 38, 0, 0, 0, 22, 0, 3, 0, 40, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 41, 0, 0, 0, 40, 0, 0, 0, 4, 0, 0, 0, 24, 0, 4, 0, 42, 0, 0, 0, 41, 0, 0, 0, 4, 0, 0, 0, 21, 0, 4, 0, 43, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 23, 0, 4, 0, 44, 0, 0, 0, 43, 0, 0, 0, 4, 0, 0, 0, 23, 0, 4, 0, 45, 0, 0, 0, 40, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 46, 0, 0, 0, 1, 0, 0, 0, 45, 0, 0, 0, 32, 0, 4, 0, 47, 0, 0, 0, 1, 0, 0, 0, 41, 0, 0, 0, 32, 0, 4, 0, 48, 0, 0, 0, 1, 0, 0, 0, 44, 0, 0, 0, 32, 0, 4, 0, 49, 0, 0, 0, 1, 0, 0, 0, 43, 0, 0, 0, 43, 0, 4, 0, 40, 0, 0, 0, 50, 0, 0, 0, 0, 0, 0, 64, 43, 0, 4, 0, 40, 0, 0, 0, 51, 0, 0, 0, 0, 0, 128, 63, 43, 0, 4, 0, 40, 0, 0, 0, 52, 0, 0, 0, 0, 0, 0, 0, 44, 0, 7, 0, 41, 0, 0, 0, 53, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 51, 0, 0, 0, 44, 0, 7, 0, 41, 0, 0, 0, 54, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 51, 0, 0, 0, 52, 0, 0, 0, 21, 0, 4, 0, 55, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 55, 0, 0, 0, 56, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 55, 0, 0, 0, 57, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 55, 0, 0, 0, 58, 0, 0, 0, 2, 0, 0, 0, 43, 0, 4, 0, 43, 0, 0, 0, 59, 0, 0, 0, 2, 0, 0, 0, 20, 0, 2, 0, 60, 0, 0, 0, 43, 0, 4, 0, 43, 0, 0, 0, 61, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 55, 0, 0, 0, 62, 0, 0, 0, 4, 0, 0, 0, 28, 0, 4, 0, 37, 0, 0, 0, 41, 0, 0, 0, 62, 0, 0, 0, 30, 0, 3, 0, 34, 0, 0, 0, 37, 0, 0, 0, 30, 0, 10, 0, 35, 0, 0, 0, 34, 0, 0, 0, 34, 0, 0, 0, 34, 0, 0, 0, 34, 0, 0, 0, 34, 0, 0, 0, 34, 0, 0, 0, 45, 0, 0, 0, 45, 0, 0, 0, 30, 0, 3, 0, 36, 0, 0, 0, 35, 0, 0, 0, 32, 0, 4, 0, 63, 0, 0, 0, 2, 0, 0, 0, 36, 0, 0, 0, 32, 0, 4, 0, 64, 0, 0, 0, 2, 0, 0, 0, 34, 0, 0, 0, 32, 0, 4, 0, 65, 0, 0, 0, 3, 0, 0, 0, 41, 0, 0, 0, 32, 0, 4, 0, 66, 0, 0, 0, 3, 0, 0, 0, 44, 0, 0, 0, 32, 0, 4, 0, 67, 0, 0, 0, 3, 0, 0, 0, 45, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 9, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 10, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 47, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 12, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 13, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 14, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 15, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 48, 0, 0, 0, 16, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 47, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 47, 0, 0, 0, 18, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 49, 0, 0, 0, 19, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 63, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 59, 0, 4, 0, 65, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 65, 0, 0, 0, 4, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 66, 0, 0, 0, 5, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 67, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 67, 0, 0, 0, 7, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 67, 0, 0, 0, 8, 0, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 38, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 39, 0, 0, 0, 248, 0, 2, 0, 68, 0, 0, 0, 61, 0, 4, 0, 45, 0, 0, 0, 69, 0, 0, 0, 9, 0, 0, 0, 61, 0, 4, 0, 45, 0, 0, 0, 70, 0, 0, 0, 10, 0, 0, 0, 61, 0, 4, 0, 41, 0, 0, 0, 71, 0, 0, 0, 11, 0, 0, 0, 61, 0, 4, 0, 45, 0, 0, 0, 72, 0, 0, 0, 12, 0, 0, 0, 61, 0, 4, 0, 45, 0, 0, 0, 73, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 45, 0, 0, 0, 74, 0, 0, 0, 14, 0, 0, 0, 61, 0, 4, 0, 45, 0, 0, 0, 75, 0, 0, 0, 15, 0, 0, 0, 61, 0, 4, 0, 44, 0, 0, 0, 76, 0, 0, 0, 16, 0, 0, 0, 61, 0, 4, 0, 41, 0, 0, 0, 77, 0, 0, 0, 17, 0, 0, 0, 61, 0, 4, 0, 41, 0, 0, 0, 78, 0, 0, 0, 18, 0, 0, 0, 61, 0, 4, 0, 43, 0, 0, 0, 79, 0, 0, 0, 19, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 80, 0, 0, 0, 71, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 20, 0, 0, 0, 80, 0, 0, 0, 80, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 81, 0, 0, 0, 71, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 21, 0, 0, 0, 81, 0, 0, 0, 81, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 82, 0, 0, 0, 71, 0, 0, 0, 2, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 22, 0, 0, 0, 82, 0, 0, 0, 82, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 23, 0, 0, 0, 80, 0, 0, 0, 82, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 24, 0, 0, 0, 80, 0, 0, 0, 81, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 25, 0, 0, 0, 81, 0, 0, 0, 82, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 83, 0, 0, 0, 71, 0, 0, 0, 3, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 26, 0, 0, 0, 83, 0, 0, 0, 80, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 27, 0, 0, 0, 83, 0, 0, 0, 81, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 28, 0, 0, 0, 83, 0, 0, 0, 82, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 84, 0, 0, 0, 21, 0, 0, 0, 22, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 85, 0, 0, 0, 50, 0, 0, 0, 84, 0, 0, 0, 131, 0, 5, 0, 40, 0, 0, 0, 86, 0, 0, 0, 51, 0, 0, 0, 85, 0, 0, 0, 131, 0, 5, 0, 40, 0, 0, 0, 87, 0, 0, 0, 24, 0, 0, 0, 28, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 88, 0, 0, 0, 50, 0, 0, 0, 87, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 89, 0, 0, 0, 23, 0, 0, 0, 27, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 90, 0, 0, 0, 50, 0, 0, 0, 89, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 91, 0, 0, 0, 86, 0, 0, 0, 88, 0, 0, 0, 90, 0, 0, 0, 52, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 92, 0, 0, 0, 24, 0, 0, 0, 28, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 93, 0, 0, 0, 50, 0, 0, 0, 92, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 94, 0, 0, 0, 20, 0, 0, 0, 22, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 95, 0, 0, 0, 50, 0, 0, 0, 94, 0, 0, 0, 131, 0, 5, 0, 40, 0, 0, 0, 96, 0, 0, 0, 51, 0, 0, 0, 95, 0, 0, 0, 131, 0, 5, 0, 40, 0, 0, 0, 97, 0, 0, 0, 25, 0, 0, 0, 26, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 98, 0, 0, 0, 50, 0, 0, 0, 97, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 99, 0, 0, 0, 93, 0, 0, 0, 96, 0, 0, 0, 98, 0, 0, 0, 52, 0, 0, 0, 131, 0, 5, 0, 40, 0, 0, 0, 100, 0, 0, 0, 23, 0, 0, 0, 27, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 101, 0, 0, 0, 50, 0, 0, 0, 100, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 102, 0, 0, 0, 25, 0, 0, 0, 26, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 103, 0, 0, 0, 50, 0, 0, 0, 102, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 104, 0, 0, 0, 20, 0, 0, 0, 21, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 105, 0, 0, 0, 50, 0, 0, 0, 104, 0, 0, 0, 131, 0, 5, 0, 40, 0, 0, 0, 106, 0, 0, 0, 51, 0, 0, 0, 105, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 107, 0, 0, 0, 101, 0, 0, 0, 103, 0, 0, 0, 106, 0, 0, 0, 52, 0, 0, 0, 80, 0, 7, 0, 42, 0, 0, 0, 29, 0, 0, 0, 91, 0, 0, 0, 99, 0, 0, 0, 107, 0, 0, 0, 53, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 108, 0, 0, 0, 70, 0, 0, 0, 0, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 109, 0, 0, 0, 51, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 108, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 110, 0, 0, 0, 70, 0, 0, 0, 1, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 111, 0, 0, 0, 52, 0, 0, 0, 51, 0, 0, 0, 52, 0, 0, 0, 110, 0, 0, 0, 80, 0, 7, 0, 42, 0, 0, 0, 112, 0, 0, 0, 109, 0, 0, 0, 111, 0, 0, 0, 54, 0, 0, 0, 53, 0, 0, 0, 146, 0, 5, 0, 42, 0, 0, 0, 113, 0, 0, 0, 29, 0, 0, 0, 112, 0, 0, 0, 127, 0, 4, 0, 45, 0, 0, 0, 114, 0, 0, 0, 73, 0, 0, 0, 133, 0, 5, 0, 45, 0, 0, 0, 30, 0, 0, 0, 114, 0, 0, 0, 72, 0, 0, 0, 81, 0, 6, 0, 40, 0, 0, 0, 115, 0, 0, 0, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 116, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 117, 0, 0, 0, 115, 0, 0, 0, 116, 0, 0, 0, 81, 0, 6, 0, 40, 0, 0, 0, 118, 0, 0, 0, 113, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 119, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 120, 0, 0, 0, 118, 0, 0, 0, 119, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 121, 0, 0, 0, 117, 0, 0, 0, 120, 0, 0, 0, 81, 0, 6, 0, 40, 0, 0, 0, 122, 0, 0, 0, 113, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 123, 0, 0, 0, 121, 0, 0, 0, 122, 0, 0, 0, 82, 0, 7, 0, 42, 0, 0, 0, 124, 0, 0, 0, 123, 0, 0, 0, 113, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 81, 0, 6, 0, 40, 0, 0, 0, 125, 0, 0, 0, 113, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 126, 0, 0, 0, 125, 0, 0, 0, 116, 0, 0, 0, 81, 0, 6, 0, 40, 0, 0, 0, 127, 0, 0, 0, 113, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 128, 0, 0, 0, 127, 0, 0, 0, 119, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 129, 0, 0, 0, 126, 0, 0, 0, 128, 0, 0, 0, 81, 0, 6, 0, 40, 0, 0, 0, 130, 0, 0, 0, 113, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 131, 0, 0, 0, 129, 0, 0, 0, 130, 0, 0, 0, 82, 0, 7, 0, 42, 0, 0, 0, 132, 0, 0, 0, 131, 0, 0, 0, 124, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0, 81, 0, 6, 0, 40, 0, 0, 0, 133, 0, 0, 0, 113, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 134, 0, 0, 0, 133, 0, 0, 0, 116, 0, 0, 0, 81, 0, 6, 0, 40, 0, 0, 0, 135, 0, 0, 0, 113, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 136, 0, 0, 0, 135, 0, 0, 0, 119, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 137, 0, 0, 0, 134, 0, 0, 0, 136, 0, 0, 0, 81, 0, 6, 0, 40, 0, 0, 0, 138, 0, 0, 0, 113, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 139, 0, 0, 0, 137, 0, 0, 0, 138, 0, 0, 0, 82, 0, 7, 0, 42, 0, 0, 0, 140, 0, 0, 0, 139, 0, 0, 0, 132, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 81, 0, 6, 0, 40, 0, 0, 0, 141, 0, 0, 0, 113, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 142, 0, 0, 0, 141, 0, 0, 0, 116, 0, 0, 0, 81, 0, 6, 0, 40, 0, 0, 0, 143, 0, 0, 0, 113, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 144, 0, 0, 0, 143, 0, 0, 0, 119, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 145, 0, 0, 0, 142, 0, 0, 0, 144, 0, 0, 0, 81, 0, 6, 0, 40, 0, 0, 0, 146, 0, 0, 0, 113, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 147, 0, 0, 0, 145, 0, 0, 0, 146, 0, 0, 0, 82, 0, 7, 0, 42, 0, 0, 0, 148, 0, 0, 0, 147, 0, 0, 0, 140, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 149, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 150, 0, 0, 0, 115, 0, 0, 0, 149, 0, 0, 0, 82, 0, 7, 0, 42, 0, 0, 0, 151, 0, 0, 0, 150, 0, 0, 0, 148, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 152, 0, 0, 0, 125, 0, 0, 0, 149, 0, 0, 0, 82, 0, 7, 0, 42, 0, 0, 0, 153, 0, 0, 0, 152, 0, 0, 0, 151, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 154, 0, 0, 0, 133, 0, 0, 0, 149, 0, 0, 0, 82, 0, 7, 0, 42, 0, 0, 0, 155, 0, 0, 0, 154, 0, 0, 0, 153, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 156, 0, 0, 0, 141, 0, 0, 0, 149, 0, 0, 0, 82, 0, 7, 0, 42, 0, 0, 0, 157, 0, 0, 0, 156, 0, 0, 0, 155, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 158, 0, 0, 0, 72, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 159, 0, 0, 0, 118, 0, 0, 0, 158, 0, 0, 0, 82, 0, 7, 0, 42, 0, 0, 0, 160, 0, 0, 0, 159, 0, 0, 0, 157, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 161, 0, 0, 0, 127, 0, 0, 0, 158, 0, 0, 0, 82, 0, 7, 0, 42, 0, 0, 0, 162, 0, 0, 0, 161, 0, 0, 0, 160, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 163, 0, 0, 0, 135, 0, 0, 0, 158, 0, 0, 0, 82, 0, 7, 0, 42, 0, 0, 0, 164, 0, 0, 0, 163, 0, 0, 0, 162, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 40, 0, 0, 0, 165, 0, 0, 0, 143, 0, 0, 0, 158, 0, 0, 0, 82, 0, 7, 0, 42, 0, 0, 0, 166, 0, 0, 0, 165, 0, 0, 0, 164, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 124, 0, 4, 0, 55, 0, 0, 0, 31, 0, 0, 0, 79, 0, 0, 0, 124, 0, 4, 0, 43, 0, 0, 0, 167, 0, 0, 0, 31, 0, 0, 0, 199, 0, 5, 0, 43, 0, 0, 0, 168, 0, 0, 0, 167, 0, 0, 0, 59, 0, 0, 0, 170, 0, 5, 0, 60, 0, 0, 0, 32, 0, 0, 0, 168, 0, 0, 0, 59, 0, 0, 0, 199, 0, 5, 0, 43, 0, 0, 0, 169, 0, 0, 0, 79, 0, 0, 0, 61, 0, 0, 0, 170, 0, 5, 0, 60, 0, 0, 0, 33, 0, 0, 0, 169, 0, 0, 0, 61, 0, 0, 0, 247, 0, 3, 0, 170, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 33, 0, 0, 0, 171, 0, 0, 0, 172, 0, 0, 0, 248, 0, 2, 0, 172, 0, 0, 0, 247, 0, 3, 0, 173, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 32, 0, 0, 0, 174, 0, 0, 0, 175, 0, 0, 0, 248, 0, 2, 0, 175, 0, 0, 0, 66, 0, 6, 0, 64, 0, 0, 0, 176, 0, 0, 0, 2, 0, 0, 0, 56, 0, 0, 0, 57, 0, 0, 0, 61, 0, 4, 0, 34, 0, 0, 0, 177, 0, 0, 0, 176, 0, 0, 0, 81, 0, 5, 0, 37, 0, 0, 0, 178, 0, 0, 0, 177, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 41, 0, 0, 0, 179, 0, 0, 0, 178, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 180, 0, 0, 0, 179, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 181, 0, 0, 0, 179, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 182, 0, 0, 0, 179, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 183, 0, 0, 0, 179, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 41, 0, 0, 0, 184, 0, 0, 0, 178, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 185, 0, 0, 0, 184, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 186, 0, 0, 0, 184, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 187, 0, 0, 0, 184, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 188, 0, 0, 0, 184, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 41, 0, 0, 0, 189, 0, 0, 0, 178, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 190, 0, 0, 0, 189, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 191, 0, 0, 0, 189, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 192, 0, 0, 0, 189, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 193, 0, 0, 0, 189, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 41, 0, 0, 0, 194, 0, 0, 0, 178, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 195, 0, 0, 0, 194, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 196, 0, 0, 0, 194, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 197, 0, 0, 0, 194, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 198, 0, 0, 0, 194, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 199, 0, 0, 0, 180, 0, 0, 0, 185, 0, 0, 0, 190, 0, 0, 0, 195, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 200, 0, 0, 0, 181, 0, 0, 0, 186, 0, 0, 0, 191, 0, 0, 0, 196, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 201, 0, 0, 0, 182, 0, 0, 0, 187, 0, 0, 0, 192, 0, 0, 0, 197, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 202, 0, 0, 0, 183, 0, 0, 0, 188, 0, 0, 0, 193, 0, 0, 0, 198, 0, 0, 0, 80, 0, 7, 0, 42, 0, 0, 0, 203, 0, 0, 0, 199, 0, 0, 0, 200, 0, 0, 0, 201, 0, 0, 0, 202, 0, 0, 0, 249, 0, 2, 0, 173, 0, 0, 0, 248, 0, 2, 0, 174, 0, 0, 0, 66, 0, 6, 0, 64, 0, 0, 0, 204, 0, 0, 0, 2, 0, 0, 0, 56, 0, 0, 0, 58, 0, 0, 0, 61, 0, 4, 0, 34, 0, 0, 0, 205, 0, 0, 0, 204, 0, 0, 0, 81, 0, 5, 0, 37, 0, 0, 0, 206, 0, 0, 0, 205, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 41, 0, 0, 0, 207, 0, 0, 0, 206, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 208, 0, 0, 0, 207, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 209, 0, 0, 0, 207, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 210, 0, 0, 0, 207, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 211, 0, 0, 0, 207, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 41, 0, 0, 0, 212, 0, 0, 0, 206, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 213, 0, 0, 0, 212, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 214, 0, 0, 0, 212, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 215, 0, 0, 0, 212, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 216, 0, 0, 0, 212, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 41, 0, 0, 0, 217, 0, 0, 0, 206, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 218, 0, 0, 0, 217, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 219, 0, 0, 0, 217, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 220, 0, 0, 0, 217, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 221, 0, 0, 0, 217, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 41, 0, 0, 0, 222, 0, 0, 0, 206, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 223, 0, 0, 0, 222, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 224, 0, 0, 0, 222, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 225, 0, 0, 0, 222, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 226, 0, 0, 0, 222, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 227, 0, 0, 0, 208, 0, 0, 0, 213, 0, 0, 0, 218, 0, 0, 0, 223, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 228, 0, 0, 0, 209, 0, 0, 0, 214, 0, 0, 0, 219, 0, 0, 0, 224, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 229, 0, 0, 0, 210, 0, 0, 0, 215, 0, 0, 0, 220, 0, 0, 0, 225, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 230, 0, 0, 0, 211, 0, 0, 0, 216, 0, 0, 0, 221, 0, 0, 0, 226, 0, 0, 0, 80, 0, 7, 0, 42, 0, 0, 0, 231, 0, 0, 0, 227, 0, 0, 0, 228, 0, 0, 0, 229, 0, 0, 0, 230, 0, 0, 0, 249, 0, 2, 0, 173, 0, 0, 0, 248, 0, 2, 0, 173, 0, 0, 0, 245, 0, 7, 0, 42, 0, 0, 0, 232, 0, 0, 0, 203, 0, 0, 0, 175, 0, 0, 0, 231, 0, 0, 0, 174, 0, 0, 0, 249, 0, 2, 0, 170, 0, 0, 0, 248, 0, 2, 0, 171, 0, 0, 0, 66, 0, 6, 0, 64, 0, 0, 0, 233, 0, 0, 0, 2, 0, 0, 0, 56, 0, 0, 0, 56, 0, 0, 0, 61, 0, 4, 0, 34, 0, 0, 0, 234, 0, 0, 0, 233, 0, 0, 0, 81, 0, 5, 0, 37, 0, 0, 0, 235, 0, 0, 0, 234, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 41, 0, 0, 0, 236, 0, 0, 0, 235, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 237, 0, 0, 0, 236, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 238, 0, 0, 0, 236, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 239, 0, 0, 0, 236, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 240, 0, 0, 0, 236, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 41, 0, 0, 0, 241, 0, 0, 0, 235, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 242, 0, 0, 0, 241, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 243, 0, 0, 0, 241, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 244, 0, 0, 0, 241, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 245, 0, 0, 0, 241, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 41, 0, 0, 0, 246, 0, 0, 0, 235, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 247, 0, 0, 0, 246, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 248, 0, 0, 0, 246, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 249, 0, 0, 0, 246, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 250, 0, 0, 0, 246, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 41, 0, 0, 0, 251, 0, 0, 0, 235, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 252, 0, 0, 0, 251, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 253, 0, 0, 0, 251, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 254, 0, 0, 0, 251, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 255, 0, 0, 0, 251, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 0, 1, 0, 0, 237, 0, 0, 0, 242, 0, 0, 0, 247, 0, 0, 0, 252, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 1, 1, 0, 0, 238, 0, 0, 0, 243, 0, 0, 0, 248, 0, 0, 0, 253, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 2, 1, 0, 0, 239, 0, 0, 0, 244, 0, 0, 0, 249, 0, 0, 0, 254, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 3, 1, 0, 0, 240, 0, 0, 0, 245, 0, 0, 0, 250, 0, 0, 0, 255, 0, 0, 0, 80, 0, 7, 0, 42, 0, 0, 0, 4, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 2, 1, 0, 0, 3, 1, 0, 0, 249, 0, 2, 0, 170, 0, 0, 0, 248, 0, 2, 0, 170, 0, 0, 0, 245, 0, 7, 0, 42, 0, 0, 0, 5, 1, 0, 0, 232, 0, 0, 0, 173, 0, 0, 0, 4, 1, 0, 0, 171, 0, 0, 0, 146, 0, 5, 0, 42, 0, 0, 0, 6, 1, 0, 0, 166, 0, 0, 0, 5, 1, 0, 0, 80, 0, 6, 0, 41, 0, 0, 0, 7, 1, 0, 0, 69, 0, 0, 0, 52, 0, 0, 0, 51, 0, 0, 0, 144, 0, 5, 0, 41, 0, 0, 0, 8, 1, 0, 0, 7, 1, 0, 0, 6, 1, 0, 0, 82, 0, 6, 0, 41, 0, 0, 0, 9, 1, 0, 0, 51, 0, 0, 0, 8, 1, 0, 0, 2, 0, 0, 0, 79, 0, 7, 0, 45, 0, 0, 0, 10, 1, 0, 0, 77, 0, 0, 0, 77, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 133, 0, 5, 0, 45, 0, 0, 0, 11, 1, 0, 0, 69, 0, 0, 0, 10, 1, 0, 0, 79, 0, 7, 0, 45, 0, 0, 0, 12, 1, 0, 0, 77, 0, 0, 0, 77, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 129, 0, 5, 0, 45, 0, 0, 0, 13, 1, 0, 0, 11, 1, 0, 0, 12, 1, 0, 0, 62, 0, 3, 0, 3, 0, 0, 0, 9, 1, 0, 0, 62, 0, 3, 0, 4, 0, 0, 0, 78, 0, 0, 0, 62, 0, 3, 0, 5, 0, 0, 0, 76, 0, 0, 0, 62, 0, 3, 0, 6, 0, 0, 0, 74, 0, 0, 0, 62, 0, 3, 0, 7, 0, 0, 0, 75, 0, 0, 0, 62, 0, 3, 0, 8, 0, 0, 0, 13, 1, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_NINEPATCH_FRAG[2968] = {3, 2, 35, 7, 0, 5, 1, 0, 0, 0, 40, 0, 121, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 13, 0, 4, 0, 0, 0, 1, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 16, 0, 3, 0, 1, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 11, 0, 0, 0, 1, 0, 0, 0, 5, 0, 5, 0, 5, 0, 0, 0, 105, 110, 112, 46, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 5, 0, 6, 0, 0, 0, 105, 110, 112, 46, 109, 97, 114, 103, 105, 110, 0, 0, 5, 0, 6, 0, 7, 0, 0, 0, 105, 110, 112, 46, 115, 111, 117, 114, 99, 101, 95, 115, 105, 122, 101, 0, 5, 0, 6, 0, 8, 0, 0, 0, 105, 110, 112, 46, 111, 117, 116, 112, 117, 116, 95, 115, 105, 122, 101, 0, 5, 0, 4, 0, 9, 0, 0, 0, 105, 110, 112, 46, 117, 118, 0, 0, 5, 0, 4, 0, 10, 0, 0, 0, 110, 101, 119, 95, 117, 118, 0, 0, 5, 0, 4, 0, 2, 0, 0, 0, 84, 101, 120, 116, 117, 114, 101, 0, 5, 0, 4, 0, 3, 0, 0, 0, 83, 97, 109, 112, 108, 101, 114, 0, 5, 0, 6, 0, 11, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 73, 109, 97, 103, 101, 0, 0, 0, 0, 5, 0, 4, 0, 12, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 0, 5, 0, 4, 0, 13, 0, 0, 0, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 7, 0, 4, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 80, 83, 0, 0, 5, 0, 3, 0, 1, 0, 0, 0, 80, 83, 0, 0, 71, 0, 4, 0, 5, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 5, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 3, 0, 6, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 7, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 3, 0, 7, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 8, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 3, 0, 8, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 9, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 2, 0, 0, 0, 33, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 2, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 33, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 4, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 14, 0, 0, 0, 33, 0, 3, 0, 15, 0, 0, 0, 14, 0, 0, 0, 22, 0, 3, 0, 16, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 17, 0, 0, 0, 16, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 18, 0, 0, 0, 1, 0, 0, 0, 17, 0, 0, 0, 21, 0, 4, 0, 19, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 23, 0, 4, 0, 20, 0, 0, 0, 19, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 21, 0, 0, 0, 1, 0, 0, 0, 20, 0, 0, 0, 23, 0, 4, 0, 22, 0, 0, 0, 16, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 23, 0, 0, 0, 1, 0, 0, 0, 22, 0, 0, 0, 23, 0, 4, 0, 24, 0, 0, 0, 19, 0, 0, 0, 2, 0, 0, 0, 20, 0, 2, 0, 25, 0, 0, 0, 43, 0, 4, 0, 16, 0, 0, 0, 26, 0, 0, 0, 0, 0, 128, 63, 25, 0, 9, 0, 27, 0, 0, 0, 16, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 28, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 0, 26, 0, 2, 0, 29, 0, 0, 0, 32, 0, 4, 0, 30, 0, 0, 0, 0, 0, 0, 0, 29, 0, 0, 0, 27, 0, 3, 0, 31, 0, 0, 0, 27, 0, 0, 0, 43, 0, 4, 0, 16, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 63, 32, 0, 4, 0, 33, 0, 0, 0, 3, 0, 0, 0, 17, 0, 0, 0, 59, 0, 4, 0, 18, 0, 0, 0, 5, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 21, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 23, 0, 0, 0, 7, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 23, 0, 0, 0, 8, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 23, 0, 0, 0, 9, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 59, 0, 4, 0, 30, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 59, 0, 4, 0, 33, 0, 0, 0, 4, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 19, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 54, 0, 5, 0, 14, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 248, 0, 2, 0, 35, 0, 0, 0, 61, 0, 4, 0, 17, 0, 0, 0, 36, 0, 0, 0, 5, 0, 0, 0, 61, 0, 4, 0, 20, 0, 0, 0, 37, 0, 0, 0, 6, 0, 0, 0, 61, 0, 4, 0, 22, 0, 0, 0, 38, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 22, 0, 0, 0, 39, 0, 0, 0, 8, 0, 0, 0, 61, 0, 4, 0, 22, 0, 0, 0, 40, 0, 0, 0, 9, 0, 0, 0, 79, 0, 7, 0, 24, 0, 0, 0, 41, 0, 0, 0, 37, 0, 0, 0, 37, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 112, 0, 4, 0, 22, 0, 0, 0, 42, 0, 0, 0, 41, 0, 0, 0, 79, 0, 7, 0, 24, 0, 0, 0, 43, 0, 0, 0, 37, 0, 0, 0, 37, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 112, 0, 4, 0, 22, 0, 0, 0, 44, 0, 0, 0, 43, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 45, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 79, 0, 7, 0, 22, 0, 0, 0, 46, 0, 0, 0, 38, 0, 0, 0, 38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 136, 0, 5, 0, 22, 0, 0, 0, 47, 0, 0, 0, 42, 0, 0, 0, 46, 0, 0, 0, 79, 0, 7, 0, 22, 0, 0, 0, 48, 0, 0, 0, 39, 0, 0, 0, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 136, 0, 5, 0, 22, 0, 0, 0, 49, 0, 0, 0, 42, 0, 0, 0, 48, 0, 0, 0, 247, 0, 3, 0, 50, 0, 0, 0, 0, 0, 0, 0, 251, 0, 3, 0, 34, 0, 0, 0, 51, 0, 0, 0, 248, 0, 2, 0, 51, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 52, 0, 0, 0, 49, 0, 0, 0, 0, 0, 0, 0, 184, 0, 5, 0, 25, 0, 0, 0, 53, 0, 0, 0, 45, 0, 0, 0, 52, 0, 0, 0, 247, 0, 3, 0, 54, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 53, 0, 0, 0, 55, 0, 0, 0, 54, 0, 0, 0, 248, 0, 2, 0, 54, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 56, 0, 0, 0, 49, 0, 0, 0, 1, 0, 0, 0, 131, 0, 5, 0, 16, 0, 0, 0, 57, 0, 0, 0, 26, 0, 0, 0, 56, 0, 0, 0, 184, 0, 5, 0, 25, 0, 0, 0, 58, 0, 0, 0, 45, 0, 0, 0, 57, 0, 0, 0, 247, 0, 3, 0, 59, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 58, 0, 0, 0, 60, 0, 0, 0, 59, 0, 0, 0, 248, 0, 2, 0, 59, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 61, 0, 0, 0, 47, 0, 0, 0, 1, 0, 0, 0, 131, 0, 5, 0, 16, 0, 0, 0, 62, 0, 0, 0, 26, 0, 0, 0, 61, 0, 0, 0, 131, 0, 5, 0, 16, 0, 0, 0, 63, 0, 0, 0, 45, 0, 0, 0, 57, 0, 0, 0, 136, 0, 5, 0, 16, 0, 0, 0, 64, 0, 0, 0, 63, 0, 0, 0, 56, 0, 0, 0, 133, 0, 5, 0, 16, 0, 0, 0, 65, 0, 0, 0, 64, 0, 0, 0, 61, 0, 0, 0, 129, 0, 5, 0, 16, 0, 0, 0, 66, 0, 0, 0, 65, 0, 0, 0, 62, 0, 0, 0, 249, 0, 2, 0, 50, 0, 0, 0, 248, 0, 2, 0, 60, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 67, 0, 0, 0, 47, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 68, 0, 0, 0, 47, 0, 0, 0, 1, 0, 0, 0, 131, 0, 5, 0, 16, 0, 0, 0, 69, 0, 0, 0, 26, 0, 0, 0, 68, 0, 0, 0, 131, 0, 5, 0, 16, 0, 0, 0, 70, 0, 0, 0, 45, 0, 0, 0, 52, 0, 0, 0, 131, 0, 5, 0, 16, 0, 0, 0, 71, 0, 0, 0, 57, 0, 0, 0, 52, 0, 0, 0, 136, 0, 5, 0, 16, 0, 0, 0, 72, 0, 0, 0, 70, 0, 0, 0, 71, 0, 0, 0, 131, 0, 5, 0, 16, 0, 0, 0, 73, 0, 0, 0, 69, 0, 0, 0, 67, 0, 0, 0, 133, 0, 5, 0, 16, 0, 0, 0, 74, 0, 0, 0, 72, 0, 0, 0, 73, 0, 0, 0, 129, 0, 5, 0, 16, 0, 0, 0, 75, 0, 0, 0, 74, 0, 0, 0, 67, 0, 0, 0, 249, 0, 2, 0, 50, 0, 0, 0, 248, 0, 2, 0, 55, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 76, 0, 0, 0, 47, 0, 0, 0, 0, 0, 0, 0, 136, 0, 5, 0, 16, 0, 0, 0, 77, 0, 0, 0, 45, 0, 0, 0, 52, 0, 0, 0, 133, 0, 5, 0, 16, 0, 0, 0, 78, 0, 0, 0, 77, 0, 0, 0, 76, 0, 0, 0, 249, 0, 2, 0, 50, 0, 0, 0, 248, 0, 2, 0, 50, 0, 0, 0, 245, 0, 9, 0, 16, 0, 0, 0, 79, 0, 0, 0, 66, 0, 0, 0, 59, 0, 0, 0, 75, 0, 0, 0, 60, 0, 0, 0, 78, 0, 0, 0, 55, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 80, 0, 0, 0, 40, 0, 0, 0, 1, 0, 0, 0, 79, 0, 7, 0, 22, 0, 0, 0, 81, 0, 0, 0, 38, 0, 0, 0, 38, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 136, 0, 5, 0, 22, 0, 0, 0, 82, 0, 0, 0, 44, 0, 0, 0, 81, 0, 0, 0, 79, 0, 7, 0, 22, 0, 0, 0, 83, 0, 0, 0, 39, 0, 0, 0, 39, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 136, 0, 5, 0, 22, 0, 0, 0, 84, 0, 0, 0, 44, 0, 0, 0, 83, 0, 0, 0, 247, 0, 3, 0, 85, 0, 0, 0, 0, 0, 0, 0, 251, 0, 3, 0, 34, 0, 0, 0, 86, 0, 0, 0, 248, 0, 2, 0, 86, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 87, 0, 0, 0, 84, 0, 0, 0, 0, 0, 0, 0, 184, 0, 5, 0, 25, 0, 0, 0, 88, 0, 0, 0, 80, 0, 0, 0, 87, 0, 0, 0, 247, 0, 3, 0, 89, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 88, 0, 0, 0, 90, 0, 0, 0, 89, 0, 0, 0, 248, 0, 2, 0, 89, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 91, 0, 0, 0, 84, 0, 0, 0, 1, 0, 0, 0, 131, 0, 5, 0, 16, 0, 0, 0, 92, 0, 0, 0, 26, 0, 0, 0, 91, 0, 0, 0, 184, 0, 5, 0, 25, 0, 0, 0, 93, 0, 0, 0, 80, 0, 0, 0, 92, 0, 0, 0, 247, 0, 3, 0, 94, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 93, 0, 0, 0, 95, 0, 0, 0, 94, 0, 0, 0, 248, 0, 2, 0, 94, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 96, 0, 0, 0, 82, 0, 0, 0, 1, 0, 0, 0, 131, 0, 5, 0, 16, 0, 0, 0, 97, 0, 0, 0, 26, 0, 0, 0, 96, 0, 0, 0, 131, 0, 5, 0, 16, 0, 0, 0, 98, 0, 0, 0, 80, 0, 0, 0, 92, 0, 0, 0, 136, 0, 5, 0, 16, 0, 0, 0, 99, 0, 0, 0, 98, 0, 0, 0, 91, 0, 0, 0, 133, 0, 5, 0, 16, 0, 0, 0, 100, 0, 0, 0, 99, 0, 0, 0, 96, 0, 0, 0, 129, 0, 5, 0, 16, 0, 0, 0, 101, 0, 0, 0, 100, 0, 0, 0, 97, 0, 0, 0, 249, 0, 2, 0, 85, 0, 0, 0, 248, 0, 2, 0, 95, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 102, 0, 0, 0, 82, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 103, 0, 0, 0, 82, 0, 0, 0, 1, 0, 0, 0, 131, 0, 5, 0, 16, 0, 0, 0, 104, 0, 0, 0, 26, 0, 0, 0, 103, 0, 0, 0, 131, 0, 5, 0, 16, 0, 0, 0, 105, 0, 0, 0, 80, 0, 0, 0, 87, 0, 0, 0, 131, 0, 5, 0, 16, 0, 0, 0, 106, 0, 0, 0, 92, 0, 0, 0, 87, 0, 0, 0, 136, 0, 5, 0, 16, 0, 0, 0, 107, 0, 0, 0, 105, 0, 0, 0, 106, 0, 0, 0, 131, 0, 5, 0, 16, 0, 0, 0, 108, 0, 0, 0, 104, 0, 0, 0, 102, 0, 0, 0, 133, 0, 5, 0, 16, 0, 0, 0, 109, 0, 0, 0, 107, 0, 0, 0, 108, 0, 0, 0, 129, 0, 5, 0, 16, 0, 0, 0, 110, 0, 0, 0, 109, 0, 0, 0, 102, 0, 0, 0, 249, 0, 2, 0, 85, 0, 0, 0, 248, 0, 2, 0, 90, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 111, 0, 0, 0, 82, 0, 0, 0, 0, 0, 0, 0, 136, 0, 5, 0, 16, 0, 0, 0, 112, 0, 0, 0, 80, 0, 0, 0, 87, 0, 0, 0, 133, 0, 5, 0, 16, 0, 0, 0, 113, 0, 0, 0, 112, 0, 0, 0, 111, 0, 0, 0, 249, 0, 2, 0, 85, 0, 0, 0, 248, 0, 2, 0, 85, 0, 0, 0, 245, 0, 9, 0, 16, 0, 0, 0, 114, 0, 0, 0, 101, 0, 0, 0, 94, 0, 0, 0, 110, 0, 0, 0, 95, 0, 0, 0, 113, 0, 0, 0, 90, 0, 0, 0, 80, 0, 5, 0, 22, 0, 0, 0, 10, 0, 0, 0, 79, 0, 0, 0, 114, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 115, 0, 0, 0, 2, 0, 0, 0, 61, 0, 4, 0, 29, 0, 0, 0, 116, 0, 0, 0, 3, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 11, 0, 0, 0, 115, 0, 0, 0, 116, 0, 0, 0, 87, 0, 6, 0, 17, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 17, 0, 0, 0, 13, 0, 0, 0, 12, 0, 0, 0, 36, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 117, 0, 0, 0, 13, 0, 0, 0, 3, 0, 0, 0, 188, 0, 5, 0, 25, 0, 0, 0, 118, 0, 0, 0, 117, 0, 0, 0, 32, 0, 0, 0, 247, 0, 3, 0, 119, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 118, 0, 0, 0, 120, 0, 0, 0, 119, 0, 0, 0, 248, 0, 2, 0, 120, 0, 0, 0, 252, 0, 1, 0, 248, 0, 2, 0, 119, 0, 0, 0, 62, 0, 3, 0, 4, 0, 0, 0, 13, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const char METAL_NINEPATCH_VERT[10074] = R"(#include <metal_stdlib>
#include <metal_math>
#include <metal_texture>
using namespace metal;
struct VS_Result_0
{
float4 position_0 [[position]];
float4 color_0 [[user(COLOR)]];
uint4 margin_0 [[user(MARGIN)]];
float2 source_size_0 [[user(SOURCESIZE)]];
float2 output_size_0 [[user(OUTPUTSIZE)]];
float2 uv_0 [[user(UV)]];
};
struct vertexInput_0
{
float2 position_1 [[attribute(0)]];
float2 i_position_0 [[attribute(1)]];
float4 i_rotation_0 [[attribute(2)]];
float2 i_size_0 [[attribute(3)]];
float2 i_offset_0 [[attribute(4)]];
float2 i_source_size_0 [[attribute(5)]];
float2 i_output_size_0 [[attribute(6)]];
uint4 i_margin_0 [[attribute(7)]];
float4 i_uv_offset_scale_0 [[attribute(8)]];
float4 i_color_0 [[attribute(9)]];
uint i_flags_0 [[attribute(10)]];
};
struct _MatrixStorage_float4x4_ColMajornatural_0
{
array<float4, int(4)> data_0;
};
struct GlobalUniforms_natural_0
{
_MatrixStorage_float4x4_ColMajornatural_0 screen_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 view_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 nonscale_view_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 nonscale_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 transform_matrix_0;
_MatrixStorage_float4x4_ColMajornatural_0 inv_view_proj_0;
float2 camera_position_0;
float2 window_size_0;
};
struct SLANG_ParameterGroup_GlobalUniformBuffer_natural_0
{
GlobalUniforms_natural_0 uniforms_0;
};
struct KernelContext_0
{
SLANG_ParameterGroup_GlobalUniformBuffer_natural_0 constant* GlobalUniformBuffer_0;
};
struct VSOutput_0
{
float4 position_2;
[[flat]] float4 color_1;
[[flat]] uint4 margin_1;
[[flat]] float2 source_size_1;
[[flat]] float2 output_size_1;
float2 uv_1;
};
[[vertex]] VS_Result_0 VS(vertexInput_0 _S1 [[stage_in]], SLANG_ParameterGroup_GlobalUniformBuffer_natural_0 constant* GlobalUniformBuffer_1 [[buffer(2)]])
{
KernelContext_0 kernelContext_0;
(&kernelContext_0)->GlobalUniformBuffer_0 = GlobalUniformBuffer_1;
float _S2 = _S1.i_rotation_0.x;
float qxx_0 = _S2 * _S2;
float _S3 = _S1.i_rotation_0.y;
float qyy_0 = _S3 * _S3;
float _S4 = _S1.i_rotation_0.z;
float qzz_0 = _S4 * _S4;
float qxz_0 = _S2 * _S4;
float qxy_0 = _S2 * _S3;
float qyz_0 = _S3 * _S4;
float _S5 = _S1.i_rotation_0.w;
float qwx_0 = _S5 * _S2;
float qwy_0 = _S5 * _S3;
float qwz_0 = _S5 * _S4;
float4 _S6 = float4(0.0, 0.0, 0.0, 1.0);
thread matrix<float,int(4),int(4)>  transform_0 = (((matrix<float,int(4),int(4)> (float4(1.0 - 2.0 * (qyy_0 + qzz_0), 2.0 * (qxy_0 - qwz_0), 2.0 * (qxz_0 + qwy_0), 0.0), float4(2.0 * (qxy_0 + qwz_0), 1.0 - 2.0 * (qxx_0 + qzz_0), 2.0 * (qyz_0 - qwx_0), 0.0), float4(2.0 * (qxz_0 - qwy_0), 2.0 * (qyz_0 + qwx_0), 1.0 - 2.0 * (qxx_0 + qyy_0), 0.0), _S6)) * (matrix<float,int(4),int(4)> (float4(1.0, 0.0, 0.0, _S1.i_position_0.x), float4(0.0, 1.0, 0.0, _S1.i_position_0.y), float4(0.0, 0.0, 1.0, 0.0), _S6))));
float2 offset_0 = - _S1.i_offset_0 * _S1.i_size_0;
transform_0[int(0)].w = transform_0[int(0)].x * offset_0[int(0)] + transform_0[int(0)].y * offset_0[int(1)] + transform_0[int(0)].z * 0.0 + transform_0[int(0)].w;
transform_0[int(1)].w = transform_0[int(1)].x * offset_0[int(0)] + transform_0[int(1)].y * offset_0[int(1)] + transform_0[int(1)].z * 0.0 + transform_0[int(1)].w;
transform_0[int(2)].w = transform_0[int(2)].x * offset_0[int(0)] + transform_0[int(2)].y * offset_0[int(1)] + transform_0[int(2)].z * 0.0 + transform_0[int(2)].w;
transform_0[int(3)].w = transform_0[int(3)].x * offset_0[int(0)] + transform_0[int(3)].y * offset_0[int(1)] + transform_0[int(3)].z * 0.0 + transform_0[int(3)].w;
transform_0[int(0)].x = transform_0[int(0)].x * _S1.i_size_0[int(0)];
transform_0[int(1)].x = transform_0[int(1)].x * _S1.i_size_0[int(0)];
transform_0[int(2)].x = transform_0[int(2)].x * _S1.i_size_0[int(0)];
transform_0[int(3)].x = transform_0[int(3)].x * _S1.i_size_0[int(0)];
transform_0[int(0)].y = transform_0[int(0)].y * _S1.i_size_0[int(1)];
transform_0[int(1)].y = transform_0[int(1)].y * _S1.i_size_0[int(1)];
transform_0[int(2)].y = transform_0[int(2)].y * _S1.i_size_0[int(1)];
transform_0[int(3)].y = transform_0[int(3)].y * _S1.i_size_0[int(1)];
bool ignore_camera_zoom_0 = (uint(int(_S1.i_flags_0)) & 2U) == 2U;
matrix<float,int(4),int(4)>  _S7;
if(((_S1.i_flags_0) & 1U) == 1U)
{
_S7 = matrix<float,int(4),int(4)> ((&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(3)]);
}
else
{
if(ignore_camera_zoom_0)
{
_S7 = matrix<float,int(4),int(4)> ((&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(0)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(1)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(2)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(3)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(0)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(1)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(2)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(3)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(0)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(1)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(2)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(3)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(0)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(1)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(2)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(3)][int(3)]);
}
else
{
_S7 = matrix<float,int(4),int(4)> ((&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(3)]);
}
}
thread VSOutput_0 outp_0;
(&outp_0)->position_2 = (((float4(_S1.position_1, 0.0, 1.0)) * ((((transform_0) * (_S7))))));
(&outp_0)->position_2.z = 1.0;
(&outp_0)->uv_1 = _S1.position_1 * _S1.i_uv_offset_scale_0.zw + _S1.i_uv_offset_scale_0.xy;
(&outp_0)->color_1 = _S1.i_color_0;
(&outp_0)->margin_1 = _S1.i_margin_0;
(&outp_0)->source_size_1 = _S1.i_source_size_0;
(&outp_0)->output_size_1 = _S1.i_output_size_0;
VSOutput_0 _S8 = outp_0;
thread VS_Result_0 _S9;
(&_S9)->position_0 = _S8.position_2;
(&_S9)->color_0 = _S8.color_1;
(&_S9)->margin_0 = _S8.margin_1;
(&_S9)->source_size_0 = _S8.source_size_1;
(&_S9)->output_size_0 = _S8.output_size_1;
(&_S9)->uv_0 = _S8.uv_1;
return _S9;
}
)";

static const char METAL_NINEPATCH_FRAG[2037] = R"(#include <metal_stdlib>
#include <metal_math>
#include <metal_texture>
using namespace metal;
float map_0(float value_0, float in_min_0, float in_max_0, float out_min_0, float out_max_0)
{
return (value_0 - in_min_0) / (in_max_0 - in_min_0) * (out_max_0 - out_min_0) + out_min_0;
}
float process_axis_0(float coord_0, const float2 thread* source_margin_0, const float2 thread* out_margin_0)
{
float _S1 = (*out_margin_0).x;
if(coord_0 < _S1)
{
return map_0(coord_0, 0.0, _S1, 0.0, (*source_margin_0).x);
}
float _S2 = 1.0 - (*out_margin_0).y;
if(coord_0 < _S2)
{
return map_0(coord_0, _S1, _S2, (*source_margin_0).x, 1.0 - (*source_margin_0).y);
}
return map_0(coord_0, _S2, 1.0, 1.0 - (*source_margin_0).y, 1.0);
}
struct pixelOutput_0
{
float4 output_0 [[color(0)]];
};
struct pixelInput_0
{
[[flat]] float4 color_0 [[user(COLOR)]];
[[flat]] uint4 margin_0 [[user(MARGIN)]];
[[flat]] float2 source_size_0 [[user(SOURCESIZE)]];
[[flat]] float2 output_size_0 [[user(OUTPUTSIZE)]];
float2 uv_0 [[user(UV)]];
};
struct KernelContext_0
{
texture2d<float, access::sample> Texture_0;
sampler Sampler_0;
};
[[fragment]] pixelOutput_0 PS(pixelInput_0 _S3 [[stage_in]], float4 position_0 [[position]], texture2d<float, access::sample> Texture_1 [[texture(3)]], sampler Sampler_1 [[sampler(4)]])
{
KernelContext_0 kernelContext_0;
(&kernelContext_0)->Texture_0 = Texture_1;
(&kernelContext_0)->Sampler_0 = Sampler_1;
float2 _S4 = float2(_S3.margin_0.xy);
float2 _S5 = float2(_S3.margin_0.zw);
float _S6 = _S3.uv_0.x;
float2 _S7 = _S4 / _S3.output_size_0.xx;
float2 _S8 = _S4 / _S3.source_size_0.xx;
float2 _S9 = _S7;
float _S10 = process_axis_0(_S6, &_S8, &_S9);
float _S11 = _S3.uv_0.y;
float2 _S12 = _S5 / _S3.output_size_0.yy;
float2 _S13 = _S5 / _S3.source_size_0.yy;
float2 _S14 = _S12;
float _S15 = process_axis_0(_S11, &_S13, &_S14);
float4 color_1 = (((&kernelContext_0)->Texture_0).sample((Sampler_1), (float2(_S10, _S15)))) * _S3.color_0;
if((color_1.w) <= 0.5)
{
discard_fragment();
}
pixelOutput_0 _S16 = { color_1 };
return _S16;
}
)";

static const char GL_NINEPATCH_VERT[6552] = R"(#version 410
struct _MatrixStorage_float4x4_ColMajorstd140
{
vec4 data[4];
};
struct GlobalUniforms_std140
{
_MatrixStorage_float4x4_ColMajorstd140 screen_projection;
_MatrixStorage_float4x4_ColMajorstd140 view_projection;
_MatrixStorage_float4x4_ColMajorstd140 nonscale_view_projection;
_MatrixStorage_float4x4_ColMajorstd140 nonscale_projection;
_MatrixStorage_float4x4_ColMajorstd140 transform_matrix;
_MatrixStorage_float4x4_ColMajorstd140 inv_view_proj;
vec2 camera_position;
vec2 window_size;
};
layout(std140) uniform GlobalUniformBuffer_std140
{
GlobalUniforms_std140 uniforms;
} GlobalUniformBuffer;
layout(location = 0) in vec2 inp_position;
layout(location = 1) in vec2 inp_i_position;
layout(location = 2) in vec4 inp_i_rotation;
layout(location = 3) in vec2 inp_i_size;
layout(location = 4) in vec2 inp_i_offset;
layout(location = 5) in vec2 inp_i_source_size;
layout(location = 6) in vec2 inp_i_output_size;
layout(location = 7) in uvec4 inp_i_margin;
layout(location = 8) in vec4 inp_i_uv_offset_scale;
layout(location = 9) in vec4 inp_i_color;
layout(location = 10) in uint inp_i_flags;
layout(location = 0) flat out vec4 entryPointParam_VS_color;
layout(location = 1) flat out uvec4 entryPointParam_VS_margin;
layout(location = 2) flat out vec2 entryPointParam_VS_source_size;
layout(location = 3) flat out vec2 entryPointParam_VS_output_size;
layout(location = 4) out vec2 entryPointParam_VS_uv;
void main()
{
float qxx = inp_i_rotation.x * inp_i_rotation.x;
float qyy = inp_i_rotation.y * inp_i_rotation.y;
float qzz = inp_i_rotation.z * inp_i_rotation.z;
float qxz = inp_i_rotation.x * inp_i_rotation.z;
float qxy = inp_i_rotation.x * inp_i_rotation.y;
float qyz = inp_i_rotation.y * inp_i_rotation.z;
float qwx = inp_i_rotation.w * inp_i_rotation.x;
float qwy = inp_i_rotation.w * inp_i_rotation.y;
float qwz = inp_i_rotation.w * inp_i_rotation.z;
mat4 _113 = mat4(vec4(1.0 - (2.0 * (qyy + qzz)), 2.0 * (qxy - qwz), 2.0 * (qxz + qwy), 0.0), vec4(2.0 * (qxy + qwz), 1.0 - (2.0 * (qxx + qzz)), 2.0 * (qyz - qwx), 0.0), vec4(2.0 * (qxz - qwy), 2.0 * (qyz + qwx), 1.0 - (2.0 * (qxx + qyy)), 0.0), vec4(0.0, 0.0, 0.0, 1.0)) * mat4(vec4(1.0, 0.0, 0.0, inp_i_position.x), vec4(0.0, 1.0, 0.0, inp_i_position.y), vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
vec2 offset = (-inp_i_offset) * inp_i_size;
float _115 = _113[0].x;
float _116 = offset.x;
float _118 = _113[0].y;
float _119 = offset.y;
mat4 _124 = _113;
_124[0].w = ((_115 * _116) + (_118 * _119)) + _113[0].w;
float _125 = _113[1].x;
float _127 = _113[1].y;
_124[1].w = ((_125 * _116) + (_127 * _119)) + _113[1].w;
float _133 = _113[2].x;
float _135 = _113[2].y;
_124[2].w = ((_133 * _116) + (_135 * _119)) + _113[2].w;
float _141 = _113[3].x;
float _143 = _113[3].y;
_124[3].w = ((_141 * _116) + (_143 * _119)) + _113[3].w;
_124[0].x = _115 * inp_i_size.x;
_124[1].x = _125 * inp_i_size.x;
_124[2].x = _133 * inp_i_size.x;
_124[3].x = _141 * inp_i_size.x;
_124[0].y = _118 * inp_i_size.y;
_124[1].y = _127 * inp_i_size.y;
_124[2].y = _135 * inp_i_size.y;
_124[3].y = _143 * inp_i_size.y;
mat4 _261;
if ((inp_i_flags & 1u) == 1u)
{
_261 = mat4(vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].x, GlobalUniformBuffer.uniforms.screen_projection.data[1].x, GlobalUniformBuffer.uniforms.screen_projection.data[2].x, GlobalUniformBuffer.uniforms.screen_projection.data[3].x), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].y, GlobalUniformBuffer.uniforms.screen_projection.data[1].y, GlobalUniformBuffer.uniforms.screen_projection.data[2].y, GlobalUniformBuffer.uniforms.screen_projection.data[3].y), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].z, GlobalUniformBuffer.uniforms.screen_projection.data[1].z, GlobalUniformBuffer.uniforms.screen_projection.data[2].z, GlobalUniformBuffer.uniforms.screen_projection.data[3].z), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].w, GlobalUniformBuffer.uniforms.screen_projection.data[1].w, GlobalUniformBuffer.uniforms.screen_projection.data[2].w, GlobalUniformBuffer.uniforms.screen_projection.data[3].w));
}
else
{
mat4 _232;
if ((uint(int(inp_i_flags)) & 2u) == 2u)
{
_232 = mat4(vec4(GlobalUniformBuffer.uniforms.nonscale_view_projection.data[0].x, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[1].x, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[2].x, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[3].x), vec4(GlobalUniformBuffer.uniforms.nonscale_view_projection.data[0].y, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[1].y, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[2].y, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[3].y), vec4(GlobalUniformBuffer.uniforms.nonscale_view_projection.data[0].z, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[1].z, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[2].z, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[3].z), vec4(GlobalUniformBuffer.uniforms.nonscale_view_projection.data[0].w, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[1].w, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[2].w, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[3].w));
}
else
{
_232 = mat4(vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].x, GlobalUniformBuffer.uniforms.view_projection.data[1].x, GlobalUniformBuffer.uniforms.view_projection.data[2].x, GlobalUniformBuffer.uniforms.view_projection.data[3].x), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].y, GlobalUniformBuffer.uniforms.view_projection.data[1].y, GlobalUniformBuffer.uniforms.view_projection.data[2].y, GlobalUniformBuffer.uniforms.view_projection.data[3].y), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].z, GlobalUniformBuffer.uniforms.view_projection.data[1].z, GlobalUniformBuffer.uniforms.view_projection.data[2].z, GlobalUniformBuffer.uniforms.view_projection.data[3].z), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].w, GlobalUniformBuffer.uniforms.view_projection.data[1].w, GlobalUniformBuffer.uniforms.view_projection.data[2].w, GlobalUniformBuffer.uniforms.view_projection.data[3].w));
}
_261 = _232;
}
vec4 _264 = vec4(inp_position, 0.0, 1.0) * (_124 * _261);
_264.z = 1.0;
gl_Position = _264;
entryPointParam_VS_color = inp_i_color;
entryPointParam_VS_margin = inp_i_margin;
entryPointParam_VS_source_size = inp_i_source_size;
entryPointParam_VS_output_size = inp_i_output_size;
entryPointParam_VS_uv = (inp_position * inp_i_uv_offset_scale.zw) + inp_i_uv_offset_scale.xy;
}
)";

static const char GL_NINEPATCH_FRAG[1418] = R"(#version 410
uniform sampler2D Texture;
layout(location = 0) flat in vec4 inp_color;
layout(location = 1) flat in uvec4 inp_margin;
layout(location = 2) flat in vec2 inp_source_size;
layout(location = 3) flat in vec2 inp_output_size;
layout(location = 4) in vec2 inp_uv;
layout(location = 0) out vec4 entryPointParam_PS;
void main()
{
vec2 _42 = vec2(inp_margin.xy);
vec2 _44 = vec2(inp_margin.zw);
vec2 _47 = _42 / inp_source_size.xx;
vec2 _49 = _42 / inp_output_size.xx;
float _79;
do
{
float _52 = _49.x;
if (inp_uv.x < _52)
{
_79 = (inp_uv.x / _52) * _47.x;
break;
}
float _56 = _49.y;
float _57 = 1.0 - _56;
if (inp_uv.x < _57)
{
float _67 = _47.x;
_79 = (((inp_uv.x - _52) / (_57 - _52)) * ((1.0 - _47.y) - _67)) + _67;
break;
}
float _61 = _47.y;
_79 = (((inp_uv.x - _57) / _56) * _61) + (1.0 - _61);
break;
} while(false);
vec2 _82 = _44 / inp_source_size.yy;
vec2 _84 = _44 / inp_output_size.yy;
float _114;
do
{
float _87 = _84.x;
if (inp_uv.y < _87)
{
_114 = (inp_uv.y / _87) * _82.x;
break;
}
float _91 = _84.y;
float _92 = 1.0 - _91;
if (inp_uv.y < _92)
{
float _102 = _82.x;
_114 = (((inp_uv.y - _87) / (_92 - _87)) * ((1.0 - _82.y) - _102)) + _102;
break;
}
float _96 = _82.y;
_114 = (((inp_uv.y - _92) / _91) * _96) + (1.0 - _96);
break;
} while(false);
vec4 sampled = texture(Texture, vec2(_79, _114));
vec4 color = sampled * inp_color;
if (color.w <= 0.5)
{
discard;
}
entryPointParam_PS = color;
}
)";

static const char D3D11_SHAPE_VERT[3780] = R"(#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#ifndef __DXC_VERSION_MAJOR
#pragma warning(disable : 3557)
#endif
struct GlobalUniforms_0
{
float4x4 screen_projection_0;
float4x4 view_projection_0;
float4x4 nonscale_view_projection_0;
float4x4 nonscale_projection_0;
float4x4 transform_matrix_0;
float4x4 inv_view_proj_0;
float2 camera_position_0;
float2 window_size_0;
};
struct SLANG_ParameterGroup_GlobalUniformBuffer_0
{
GlobalUniforms_0 uniforms_0;
};
cbuffer GlobalUniformBuffer_0 : register(b2)
{
SLANG_ParameterGroup_GlobalUniformBuffer_0 GlobalUniformBuffer_0;
}
struct VSOutput_0
{
float4 position_0 : SV_Position;
float2 uv_0 : UV;
float2 p_0 : Point;
nointerpolation float2 size_0 : Size;
nointerpolation float4 color_0 : Color;
nointerpolation float4 border_color_0 : BorderColor;
nointerpolation float4 border_radius_0 : BorderRadius;
nointerpolation float border_thickness_0 : BorderThickness;
nointerpolation uint shape_0 : Shape;
};
struct VSInput_0
{
float2 position_1 : Position;
float3 i_position_0 : I_Position;
float2 i_size_0 : I_Size;
float2 i_offset_0 : I_Offset;
float4 i_color_0 : I_Color;
float4 i_border_color_0 : I_BorderColor;
float4 i_border_radius_0 : I_BorderRadius;
float i_border_thickness_0 : I_BorderThickness;
uint i_shape_0 : I_Shape;
uint i_flags_0 : I_Flags;
};
VSOutput_0 VS(VSInput_0 inp_0)
{
VSInput_0 _S1 = inp_0;
float4x4 transform_0 = float4x4(float4(1.0f, 0.0f, 0.0f, inp_0.i_position_0.x), float4(0.0f, 1.0f, 0.0f, inp_0.i_position_0.y), float4(0.0f, 0.0f, 1.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
float2 offset_0 = - inp_0.i_offset_0 * inp_0.i_size_0;
transform_0[int(0)][int(3)] = transform_0[int(0)][int(0)] * offset_0[int(0)] + transform_0[int(0)][int(1)] * offset_0[int(1)] + transform_0[int(0)][int(2)] * 0.0f + transform_0[int(0)][int(3)];
transform_0[int(1)][int(3)] = transform_0[int(1)][int(0)] * offset_0[int(0)] + transform_0[int(1)][int(1)] * offset_0[int(1)] + transform_0[int(1)][int(2)] * 0.0f + transform_0[int(1)][int(3)];
transform_0[int(2)][int(3)] = transform_0[int(2)][int(0)] * offset_0[int(0)] + transform_0[int(2)][int(1)] * offset_0[int(1)] + transform_0[int(2)][int(2)] * 0.0f + transform_0[int(2)][int(3)];
transform_0[int(3)][int(3)] = transform_0[int(3)][int(0)] * offset_0[int(0)] + transform_0[int(3)][int(1)] * offset_0[int(1)] + transform_0[int(3)][int(2)] * 0.0f + transform_0[int(3)][int(3)];
transform_0[int(0)][int(0)] = transform_0[int(0)][int(0)] * inp_0.i_size_0[int(0)];
transform_0[int(1)][int(0)] = transform_0[int(1)][int(0)] * inp_0.i_size_0[int(0)];
transform_0[int(2)][int(0)] = transform_0[int(2)][int(0)] * inp_0.i_size_0[int(0)];
transform_0[int(3)][int(0)] = transform_0[int(3)][int(0)] * inp_0.i_size_0[int(0)];
transform_0[int(0)][int(1)] = transform_0[int(0)][int(1)] * inp_0.i_size_0[int(1)];
transform_0[int(1)][int(1)] = transform_0[int(1)][int(1)] * inp_0.i_size_0[int(1)];
transform_0[int(2)][int(1)] = transform_0[int(2)][int(1)] * inp_0.i_size_0[int(1)];
transform_0[int(3)][int(1)] = transform_0[int(3)][int(1)] * inp_0.i_size_0[int(1)];
float4x4 _S2;
if(((inp_0.i_flags_0) & 1U) == 1U)
{
_S2 = GlobalUniformBuffer_0.uniforms_0.screen_projection_0;
}
else
{
_S2 = GlobalUniformBuffer_0.uniforms_0.view_projection_0;
}
VSOutput_0 outp_0;
outp_0.position_0 = mul(mul(_S2, transform_0), float4(_S1.position_1, 0.0f, 1.0f));
outp_0.position_0[int(2)] = _S1.i_position_0.z;
outp_0.uv_0 = _S1.position_1;
outp_0.p_0 = (_S1.position_1 - 0.5f) * _S1.i_size_0;
outp_0.size_0 = _S1.i_size_0;
outp_0.color_0 = _S1.i_color_0;
outp_0.border_color_0 = _S1.i_border_color_0;
outp_0.border_thickness_0 = _S1.i_border_thickness_0;
outp_0.border_radius_0 = _S1.i_border_radius_0;
outp_0.shape_0 = _S1.i_shape_0;
return outp_0;
}
)";

static const char D3D11_SHAPE_FRAG[5208] = R"(#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#ifndef __DXC_VERSION_MAJOR
#pragma warning(disable : 3557)
#endif
float sdf_circle_0(float2 p_0, float radius_0)
{
return length(p_0) - radius_0;
}
float antialias_circle_0(float d_0)
{
return 1.0f - smoothstep(0.0f, (fwidth((d_0))) * 1.29999995231628418f, d_0);
}
float mod_0(float x_0, float y_0)
{
return x_0 - y_0 * floor(x_0 / y_0);
}
float arc_sdf_0(float2 p_1, float a0_0, float a1_0, float r_0)
{
float ap_0 = mod_0(atan2(p_1.y, p_1.x), 6.28318548202514648f) - a0_0;
float ap_1;
if(ap_0 < 0.0f)
{
ap_1 = ap_0 + 6.28318548202514648f;
}
else
{
ap_1 = ap_0;
}
float a1p_0 = a1_0 - a0_0;
float a1p_1;
if(a1p_0 < 0.0f)
{
a1p_1 = a1p_0 + 6.28318548202514648f;
}
else
{
a1p_1 = a1p_0;
}
if(ap_1 >= a1p_1)
{
return min(length(p_1 - float2(r_0 * cos(a0_0), r_0 * sin(a0_0))), length(p_1 - float2(r_0 * cos(a1_0), r_0 * sin(a1_0))));
}
return abs(length(p_1) - r_0);
}
float sd_rounded_box_0(float2 p_2, float2 size_0, float4 corner_radii_0)
{
float2 rs_0;
if(0.0f < (p_2.y))
{
rs_0 = corner_radii_0.zw;
}
else
{
rs_0 = corner_radii_0.xy;
}
float radius_1;
if(0.0f < (p_2.x))
{
radius_1 = rs_0.y;
}
else
{
radius_1 = rs_0.x;
}
float2 q_0 = abs(p_2) - 0.5f * size_0 + radius_1;
return length(max(q_0, float2(0.0f, 0.0f))) + min(max(q_0.x, q_0.y), 0.0f) - radius_1;
}
float sd_inset_rounded_box_0(float2 p_3, float2 size_1, float4 radius_2, float4 inset_0)
{
float2 _S1 = inset_0.xy;
float2 inner_size_0 = size_1 - _S1 - inset_0.zw;
float2 inner_point_0 = p_3 - (_S1 + 0.5f * inner_size_0 - 0.5f * size_1);
float4 r_1 = radius_2;
float _S2 = inset_0.x;
float _S3 = inset_0.y;
r_1[int(0)] = radius_2.x - max(_S2, _S3);
float _S4 = inset_0.z;
r_1[int(1)] = r_1.y - max(_S4, _S3);
float _S5 = inset_0.w;
r_1[int(2)] = r_1.z - max(_S4, _S5);
r_1[int(3)] = r_1.w - max(_S2, _S5);
float2 half_size_0 = inner_size_0 * 0.5f;
float min_size_0 = min(half_size_0.x, half_size_0.y);
float4 _S6 = min(max(r_1, float4(0.0f, 0.0f, 0.0f, 0.0f)), float4(min_size_0, min_size_0, min_size_0, min_size_0));
r_1 = _S6;
return sd_rounded_box_0(inner_point_0, inner_size_0, _S6);
}
float antialias_0(float d_1)
{
return 1.0f - smoothstep(0.0f, pow((fwidth((d_1))), 0.45454543828964233f), d_1);
}
struct VSOutput_0
{
float4 position_0 : SV_Position;
float2 uv_0 : UV;
float2 p_4 : Point;
nointerpolation float2 size_2 : Size;
nointerpolation float4 color_0 : Color;
nointerpolation float4 border_color_0 : BorderColor;
nointerpolation float4 border_radius_0 : BorderRadius;
nointerpolation float border_thickness_0 : BorderThickness;
nointerpolation uint shape_0 : Shape;
};
float4 PS(VSOutput_0 inp_0) : SV_TARGET
{
VSOutput_0 _S7 = inp_0;
bool _S8;
float4 result_0;
if((inp_0.shape_0) == 1U)
{
float _S9 = _S7.size_2.x;
float radius_3 = 0.5f - 1.0f / _S9;
float2 _S10 = _S7.uv_0 - 0.5f;
float external_distance_0 = sdf_circle_0(_S10, radius_3);
float alpha_0 = antialias_circle_0(external_distance_0);
if((_S7.border_thickness_0) > 0.0f)
{
float internal_distance_0 = sdf_circle_0(_S10, radius_3 - _S7.border_thickness_0 / _S9);
float border_alpha_0 = antialias_circle_0(max(external_distance_0, - internal_distance_0));
float4 color_with_border_0 = lerp(float4(_S7.color_0.xyz, min(_S7.color_0.w, alpha_0)), _S7.border_color_0, (float4)min(_S7.border_color_0.w, min(border_alpha_0, alpha_0)));
if(internal_distance_0 > 0.0f)
{
_S8 = border_alpha_0 < 1.0f;
}
else
{
_S8 = false;
}
if(_S8)
{
result_0 = lerp(float4(0.0f, 0.0f, 0.0f, 0.0f), _S7.border_color_0, (float4)border_alpha_0);
}
else
{
result_0 = color_with_border_0;
}
}
else
{
result_0 = float4(_S7.color_0.xyz, min(_S7.color_0.w, alpha_0));
}
}
else
{
if((_S7.shape_0) == 2U)
{
float _S11 = _S7.size_2.y;
float thickness_0 = 0.5f - _S7.border_thickness_0 / _S11;
float d_2 = arc_sdf_0((float2(_S7.uv_0.x, 1.0f - _S7.uv_0.y) * 2.0f - 1.0f) * _S7.size_2 / _S11, _S7.border_radius_0.x, _S7.border_radius_0.y, 1.0f - thickness_0);
return float4(_S7.color_0.xyz, min(_S7.color_0.w, 1.0f - smoothstep(thickness_0 - length(float2(ddx(d_2), ddy(d_2))), thickness_0, d_2)));
}
else
{
if((max(_S7.border_radius_0.x, max(_S7.border_radius_0.y, max(_S7.border_radius_0.z, _S7.border_radius_0.w)))) > 0.0f)
{
_S8 = true;
}
else
{
_S8 = (_S7.border_thickness_0) > 0.0f;
}
if(_S8)
{
float external_distance_1 = sd_rounded_box_0(_S7.p_4, _S7.size_2, _S7.border_radius_0);
float _S12 = _S7.border_thickness_0;
float internal_distance_1 = sd_inset_rounded_box_0(_S7.p_4, _S7.size_2, _S7.border_radius_0, float4(_S12, _S12, _S12, _S12));
float border_alpha_1 = antialias_0(max(external_distance_1, - internal_distance_1));
float smoothed_alpha_0 = antialias_0(external_distance_1);
float4 quad_color_with_border_0 = lerp(float4(_S7.color_0.xyz, min(_S7.color_0.w, smoothed_alpha_0)), _S7.border_color_0, (float4)min(_S7.border_color_0.w, min(border_alpha_1, smoothed_alpha_0)));
if(internal_distance_1 > 0.0f)
{
_S8 = border_alpha_1 < 1.0f;
}
else
{
_S8 = false;
}
if(_S8)
{
result_0 = float4(_S7.border_color_0.xyz, border_alpha_1);
}
else
{
result_0 = quad_color_with_border_0;
}
}
else
{
result_0 = _S7.color_0;
}
}
}
if((result_0.w) <= 0.00999999977648258f)
{
discard;
}
return result_0;
}
)";

static const unsigned char VULKAN_SHAPE_VERT[5124] = {3, 2, 35, 7, 0, 5, 1, 0, 0, 0, 40, 0, 162, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 25, 0, 0, 0, 0, 0, 1, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 10, 0, 0, 0, 11, 0, 0, 0, 12, 0, 0, 0, 13, 0, 0, 0, 14, 0, 0, 0, 15, 0, 0, 0, 16, 0, 0, 0, 17, 0, 0, 0, 18, 0, 0, 0, 19, 0, 0, 0, 20, 0, 0, 0, 21, 0, 0, 0, 3, 0, 3, 0, 11, 0, 0, 0, 1, 0, 0, 0, 5, 0, 6, 0, 12, 0, 0, 0, 105, 110, 112, 46, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 0, 0, 5, 0, 6, 0, 13, 0, 0, 0, 105, 110, 112, 46, 105, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 5, 0, 14, 0, 0, 0, 105, 110, 112, 46, 105, 95, 115, 105, 122, 101, 0, 0, 5, 0, 6, 0, 15, 0, 0, 0, 105, 110, 112, 46, 105, 95, 111, 102, 102, 115, 101, 116, 0, 0, 0, 0, 5, 0, 5, 0, 16, 0, 0, 0, 105, 110, 112, 46, 105, 95, 99, 111, 108, 111, 114, 0, 5, 0, 7, 0, 17, 0, 0, 0, 105, 110, 112, 46, 105, 95, 98, 111, 114, 100, 101, 114, 95, 99, 111, 108, 111, 114, 0, 0, 5, 0, 7, 0, 18, 0, 0, 0, 105, 110, 112, 46, 105, 95, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 5, 0, 8, 0, 19, 0, 0, 0, 105, 110, 112, 46, 105, 95, 98, 111, 114, 100, 101, 114, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 0, 5, 0, 5, 0, 20, 0, 0, 0, 105, 110, 112, 46, 105, 95, 115, 104, 97, 112, 101, 0, 5, 0, 5, 0, 21, 0, 0, 0, 105, 110, 112, 46, 105, 95, 102, 108, 97, 103, 115, 0, 5, 0, 4, 0, 22, 0, 0, 0, 111, 102, 102, 115, 101, 116, 0, 0, 5, 0, 4, 0, 23, 0, 0, 0, 105, 115, 95, 117, 105, 0, 0, 0, 5, 0, 12, 0, 24, 0, 0, 0, 95, 77, 97, 116, 114, 105, 120, 83, 116, 111, 114, 97, 103, 101, 95, 102, 108, 111, 97, 116, 52, 120, 52, 95, 67, 111, 108, 77, 97, 106, 111, 114, 115, 116, 100, 49, 52, 48, 0, 0, 6, 0, 5, 0, 24, 0, 0, 0, 0, 0, 0, 0, 100, 97, 116, 97, 0, 0, 0, 0, 5, 0, 8, 0, 25, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 115, 95, 115, 116, 100, 49, 52, 48, 0, 0, 0, 6, 0, 8, 0, 25, 0, 0, 0, 0, 0, 0, 0, 115, 99, 114, 101, 101, 110, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 7, 0, 25, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 10, 0, 25, 0, 0, 0, 2, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 0, 6, 0, 8, 0, 25, 0, 0, 0, 3, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 8, 0, 25, 0, 0, 0, 4, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 95, 109, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 7, 0, 25, 0, 0, 0, 5, 0, 0, 0, 105, 110, 118, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 0, 0, 0, 6, 0, 7, 0, 25, 0, 0, 0, 6, 0, 0, 0, 99, 97, 109, 101, 114, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 6, 0, 25, 0, 0, 0, 7, 0, 0, 0, 119, 105, 110, 100, 111, 119, 95, 115, 105, 122, 101, 0, 5, 0, 14, 0, 26, 0, 0, 0, 83, 76, 65, 78, 71, 95, 80, 97, 114, 97, 109, 101, 116, 101, 114, 71, 114, 111, 117, 112, 95, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 95, 115, 116, 100, 49, 52, 48, 0, 6, 0, 6, 0, 26, 0, 0, 0, 0, 0, 0, 0, 117, 110, 105, 102, 111, 114, 109, 115, 0, 0, 0, 0, 5, 0, 7, 0, 2, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 5, 0, 8, 0, 4, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 117, 118, 0, 0, 0, 5, 0, 8, 0, 5, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 112, 0, 0, 0, 0, 5, 0, 8, 0, 6, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 115, 105, 122, 101, 0, 5, 0, 9, 0, 7, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 99, 111, 108, 111, 114, 0, 0, 0, 0, 5, 0, 10, 0, 8, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 98, 111, 114, 100, 101, 114, 95, 99, 111, 108, 111, 114, 0, 5, 0, 11, 0, 9, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 0, 0, 0, 5, 0, 11, 0, 10, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 98, 111, 114, 100, 101, 114, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 5, 0, 9, 0, 11, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 115, 104, 97, 112, 101, 0, 0, 0, 0, 5, 0, 3, 0, 1, 0, 0, 0, 86, 83, 0, 0, 71, 0, 4, 0, 12, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 13, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 14, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 15, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 16, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 17, 0, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 4, 0, 18, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 4, 0, 19, 0, 0, 0, 30, 0, 0, 0, 7, 0, 0, 0, 71, 0, 4, 0, 20, 0, 0, 0, 30, 0, 0, 0, 8, 0, 0, 0, 71, 0, 4, 0, 21, 0, 0, 0, 30, 0, 0, 0, 9, 0, 0, 0, 71, 0, 4, 0, 27, 0, 0, 0, 6, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 24, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 25, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 25, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 5, 0, 25, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 72, 0, 5, 0, 25, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 192, 0, 0, 0, 72, 0, 5, 0, 25, 0, 0, 0, 4, 0, 0, 0, 35, 0, 0, 0, 0, 1, 0, 0, 72, 0, 5, 0, 25, 0, 0, 0, 5, 0, 0, 0, 35, 0, 0, 0, 64, 1, 0, 0, 72, 0, 5, 0, 25, 0, 0, 0, 6, 0, 0, 0, 35, 0, 0, 0, 128, 1, 0, 0, 72, 0, 5, 0, 25, 0, 0, 0, 7, 0, 0, 0, 35, 0, 0, 0, 136, 1, 0, 0, 71, 0, 3, 0, 26, 0, 0, 0, 2, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 2, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 2, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 4, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 5, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 3, 0, 6, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 7, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 3, 0, 7, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 8, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 3, 0, 8, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 9, 0, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 3, 0, 9, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 10, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 3, 0, 10, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 7, 0, 0, 0, 71, 0, 3, 0, 11, 0, 0, 0, 14, 0, 0, 0, 19, 0, 2, 0, 28, 0, 0, 0, 33, 0, 3, 0, 29, 0, 0, 0, 28, 0, 0, 0, 22, 0, 3, 0, 30, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 31, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 24, 0, 4, 0, 32, 0, 0, 0, 31, 0, 0, 0, 4, 0, 0, 0, 23, 0, 4, 0, 33, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 21, 0, 4, 0, 34, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 35, 0, 0, 0, 1, 0, 0, 0, 33, 0, 0, 0, 23, 0, 4, 0, 36, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 37, 0, 0, 0, 1, 0, 0, 0, 36, 0, 0, 0, 32, 0, 4, 0, 38, 0, 0, 0, 1, 0, 0, 0, 31, 0, 0, 0, 32, 0, 4, 0, 39, 0, 0, 0, 1, 0, 0, 0, 30, 0, 0, 0, 32, 0, 4, 0, 40, 0, 0, 0, 1, 0, 0, 0, 34, 0, 0, 0, 43, 0, 4, 0, 30, 0, 0, 0, 41, 0, 0, 0, 0, 0, 128, 63, 43, 0, 4, 0, 30, 0, 0, 0, 42, 0, 0, 0, 0, 0, 0, 0, 44, 0, 7, 0, 31, 0, 0, 0, 43, 0, 0, 0, 42, 0, 0, 0, 42, 0, 0, 0, 41, 0, 0, 0, 42, 0, 0, 0, 44, 0, 7, 0, 31, 0, 0, 0, 44, 0, 0, 0, 42, 0, 0, 0, 42, 0, 0, 0, 42, 0, 0, 0, 41, 0, 0, 0, 21, 0, 4, 0, 45, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 45, 0, 0, 0, 46, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 45, 0, 0, 0, 47, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 34, 0, 0, 0, 48, 0, 0, 0, 1, 0, 0, 0, 20, 0, 2, 0, 49, 0, 0, 0, 43, 0, 4, 0, 45, 0, 0, 0, 50, 0, 0, 0, 4, 0, 0, 0, 28, 0, 4, 0, 27, 0, 0, 0, 31, 0, 0, 0, 50, 0, 0, 0, 30, 0, 3, 0, 24, 0, 0, 0, 27, 0, 0, 0, 30, 0, 10, 0, 25, 0, 0, 0, 24, 0, 0, 0, 24, 0, 0, 0, 24, 0, 0, 0, 24, 0, 0, 0, 24, 0, 0, 0, 24, 0, 0, 0, 33, 0, 0, 0, 33, 0, 0, 0, 30, 0, 3, 0, 26, 0, 0, 0, 25, 0, 0, 0, 32, 0, 4, 0, 51, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 32, 0, 4, 0, 52, 0, 0, 0, 2, 0, 0, 0, 24, 0, 0, 0, 43, 0, 4, 0, 30, 0, 0, 0, 53, 0, 0, 0, 0, 0, 0, 63, 32, 0, 4, 0, 54, 0, 0, 0, 3, 0, 0, 0, 31, 0, 0, 0, 32, 0, 4, 0, 55, 0, 0, 0, 3, 0, 0, 0, 33, 0, 0, 0, 32, 0, 4, 0, 56, 0, 0, 0, 3, 0, 0, 0, 30, 0, 0, 0, 32, 0, 4, 0, 57, 0, 0, 0, 3, 0, 0, 0, 34, 0, 0, 0, 59, 0, 4, 0, 35, 0, 0, 0, 12, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 37, 0, 0, 0, 13, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 35, 0, 0, 0, 14, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 35, 0, 0, 0, 15, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 38, 0, 0, 0, 16, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 38, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 38, 0, 0, 0, 18, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 39, 0, 0, 0, 19, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 40, 0, 0, 0, 20, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 40, 0, 0, 0, 21, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 51, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 59, 0, 4, 0, 54, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 55, 0, 0, 0, 4, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 55, 0, 0, 0, 5, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 55, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 54, 0, 0, 0, 7, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 54, 0, 0, 0, 8, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 54, 0, 0, 0, 9, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 56, 0, 0, 0, 10, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 57, 0, 0, 0, 11, 0, 0, 0, 3, 0, 0, 0, 44, 0, 5, 0, 33, 0, 0, 0, 58, 0, 0, 0, 53, 0, 0, 0, 53, 0, 0, 0, 54, 0, 5, 0, 28, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 29, 0, 0, 0, 248, 0, 2, 0, 59, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 60, 0, 0, 0, 12, 0, 0, 0, 61, 0, 4, 0, 36, 0, 0, 0, 61, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 62, 0, 0, 0, 14, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 63, 0, 0, 0, 15, 0, 0, 0, 61, 0, 4, 0, 31, 0, 0, 0, 64, 0, 0, 0, 16, 0, 0, 0, 61, 0, 4, 0, 31, 0, 0, 0, 65, 0, 0, 0, 17, 0, 0, 0, 61, 0, 4, 0, 31, 0, 0, 0, 66, 0, 0, 0, 18, 0, 0, 0, 61, 0, 4, 0, 30, 0, 0, 0, 67, 0, 0, 0, 19, 0, 0, 0, 61, 0, 4, 0, 34, 0, 0, 0, 68, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 34, 0, 0, 0, 69, 0, 0, 0, 21, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 70, 0, 0, 0, 61, 0, 0, 0, 0, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 71, 0, 0, 0, 41, 0, 0, 0, 42, 0, 0, 0, 42, 0, 0, 0, 70, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 72, 0, 0, 0, 61, 0, 0, 0, 1, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 73, 0, 0, 0, 42, 0, 0, 0, 41, 0, 0, 0, 42, 0, 0, 0, 72, 0, 0, 0, 80, 0, 7, 0, 32, 0, 0, 0, 74, 0, 0, 0, 71, 0, 0, 0, 73, 0, 0, 0, 43, 0, 0, 0, 44, 0, 0, 0, 127, 0, 4, 0, 33, 0, 0, 0, 75, 0, 0, 0, 63, 0, 0, 0, 133, 0, 5, 0, 33, 0, 0, 0, 22, 0, 0, 0, 75, 0, 0, 0, 62, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 76, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 77, 0, 0, 0, 22, 0, 0, 0, 1, 0, 0, 0, 129, 0, 5, 0, 30, 0, 0, 0, 78, 0, 0, 0, 76, 0, 0, 0, 70, 0, 0, 0, 82, 0, 7, 0, 32, 0, 0, 0, 79, 0, 0, 0, 78, 0, 0, 0, 74, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 30, 0, 0, 0, 80, 0, 0, 0, 77, 0, 0, 0, 72, 0, 0, 0, 82, 0, 7, 0, 32, 0, 0, 0, 81, 0, 0, 0, 80, 0, 0, 0, 79, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0, 82, 0, 7, 0, 32, 0, 0, 0, 82, 0, 0, 0, 42, 0, 0, 0, 81, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 82, 0, 7, 0, 32, 0, 0, 0, 83, 0, 0, 0, 41, 0, 0, 0, 82, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 84, 0, 0, 0, 62, 0, 0, 0, 0, 0, 0, 0, 82, 0, 7, 0, 32, 0, 0, 0, 85, 0, 0, 0, 84, 0, 0, 0, 83, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 82, 0, 7, 0, 32, 0, 0, 0, 86, 0, 0, 0, 42, 0, 0, 0, 85, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 82, 0, 7, 0, 32, 0, 0, 0, 87, 0, 0, 0, 42, 0, 0, 0, 86, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 82, 0, 7, 0, 32, 0, 0, 0, 88, 0, 0, 0, 42, 0, 0, 0, 87, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 89, 0, 0, 0, 62, 0, 0, 0, 1, 0, 0, 0, 82, 0, 7, 0, 32, 0, 0, 0, 90, 0, 0, 0, 42, 0, 0, 0, 88, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 82, 0, 7, 0, 32, 0, 0, 0, 91, 0, 0, 0, 89, 0, 0, 0, 90, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 82, 0, 7, 0, 32, 0, 0, 0, 92, 0, 0, 0, 42, 0, 0, 0, 91, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 82, 0, 7, 0, 32, 0, 0, 0, 93, 0, 0, 0, 42, 0, 0, 0, 92, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 199, 0, 5, 0, 34, 0, 0, 0, 94, 0, 0, 0, 69, 0, 0, 0, 48, 0, 0, 0, 170, 0, 5, 0, 49, 0, 0, 0, 23, 0, 0, 0, 94, 0, 0, 0, 48, 0, 0, 0, 247, 0, 3, 0, 95, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 23, 0, 0, 0, 96, 0, 0, 0, 97, 0, 0, 0, 248, 0, 2, 0, 97, 0, 0, 0, 66, 0, 6, 0, 52, 0, 0, 0, 98, 0, 0, 0, 2, 0, 0, 0, 46, 0, 0, 0, 47, 0, 0, 0, 61, 0, 4, 0, 24, 0, 0, 0, 99, 0, 0, 0, 98, 0, 0, 0, 81, 0, 5, 0, 27, 0, 0, 0, 100, 0, 0, 0, 99, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 101, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 102, 0, 0, 0, 101, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 103, 0, 0, 0, 101, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 104, 0, 0, 0, 101, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 105, 0, 0, 0, 101, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 106, 0, 0, 0, 100, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 107, 0, 0, 0, 106, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 108, 0, 0, 0, 106, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 109, 0, 0, 0, 106, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 110, 0, 0, 0, 106, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 111, 0, 0, 0, 100, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 112, 0, 0, 0, 111, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 113, 0, 0, 0, 111, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 114, 0, 0, 0, 111, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 115, 0, 0, 0, 111, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 116, 0, 0, 0, 100, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 117, 0, 0, 0, 116, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 118, 0, 0, 0, 116, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 119, 0, 0, 0, 116, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 120, 0, 0, 0, 116, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 121, 0, 0, 0, 102, 0, 0, 0, 107, 0, 0, 0, 112, 0, 0, 0, 117, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 122, 0, 0, 0, 103, 0, 0, 0, 108, 0, 0, 0, 113, 0, 0, 0, 118, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 123, 0, 0, 0, 104, 0, 0, 0, 109, 0, 0, 0, 114, 0, 0, 0, 119, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 124, 0, 0, 0, 105, 0, 0, 0, 110, 0, 0, 0, 115, 0, 0, 0, 120, 0, 0, 0, 80, 0, 7, 0, 32, 0, 0, 0, 125, 0, 0, 0, 121, 0, 0, 0, 122, 0, 0, 0, 123, 0, 0, 0, 124, 0, 0, 0, 249, 0, 2, 0, 95, 0, 0, 0, 248, 0, 2, 0, 96, 0, 0, 0, 66, 0, 6, 0, 52, 0, 0, 0, 126, 0, 0, 0, 2, 0, 0, 0, 46, 0, 0, 0, 46, 0, 0, 0, 61, 0, 4, 0, 24, 0, 0, 0, 127, 0, 0, 0, 126, 0, 0, 0, 81, 0, 5, 0, 27, 0, 0, 0, 128, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 129, 0, 0, 0, 128, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 130, 0, 0, 0, 129, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 131, 0, 0, 0, 129, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 132, 0, 0, 0, 129, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 133, 0, 0, 0, 129, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 134, 0, 0, 0, 128, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 135, 0, 0, 0, 134, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 136, 0, 0, 0, 134, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 137, 0, 0, 0, 134, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 138, 0, 0, 0, 134, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 139, 0, 0, 0, 128, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 140, 0, 0, 0, 139, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 141, 0, 0, 0, 139, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 142, 0, 0, 0, 139, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 143, 0, 0, 0, 139, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 31, 0, 0, 0, 144, 0, 0, 0, 128, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 145, 0, 0, 0, 144, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 146, 0, 0, 0, 144, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 147, 0, 0, 0, 144, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 148, 0, 0, 0, 144, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 149, 0, 0, 0, 130, 0, 0, 0, 135, 0, 0, 0, 140, 0, 0, 0, 145, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 150, 0, 0, 0, 131, 0, 0, 0, 136, 0, 0, 0, 141, 0, 0, 0, 146, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 151, 0, 0, 0, 132, 0, 0, 0, 137, 0, 0, 0, 142, 0, 0, 0, 147, 0, 0, 0, 80, 0, 7, 0, 31, 0, 0, 0, 152, 0, 0, 0, 133, 0, 0, 0, 138, 0, 0, 0, 143, 0, 0, 0, 148, 0, 0, 0, 80, 0, 7, 0, 32, 0, 0, 0, 153, 0, 0, 0, 149, 0, 0, 0, 150, 0, 0, 0, 151, 0, 0, 0, 152, 0, 0, 0, 249, 0, 2, 0, 95, 0, 0, 0, 248, 0, 2, 0, 95, 0, 0, 0, 245, 0, 7, 0, 32, 0, 0, 0, 154, 0, 0, 0, 125, 0, 0, 0, 97, 0, 0, 0, 153, 0, 0, 0, 96, 0, 0, 0, 146, 0, 5, 0, 32, 0, 0, 0, 155, 0, 0, 0, 93, 0, 0, 0, 154, 0, 0, 0, 80, 0, 6, 0, 31, 0, 0, 0, 156, 0, 0, 0, 60, 0, 0, 0, 42, 0, 0, 0, 41, 0, 0, 0, 144, 0, 5, 0, 31, 0, 0, 0, 157, 0, 0, 0, 156, 0, 0, 0, 155, 0, 0, 0, 81, 0, 5, 0, 30, 0, 0, 0, 158, 0, 0, 0, 61, 0, 0, 0, 2, 0, 0, 0, 82, 0, 6, 0, 31, 0, 0, 0, 159, 0, 0, 0, 158, 0, 0, 0, 157, 0, 0, 0, 2, 0, 0, 0, 131, 0, 5, 0, 33, 0, 0, 0, 160, 0, 0, 0, 60, 0, 0, 0, 58, 0, 0, 0, 133, 0, 5, 0, 33, 0, 0, 0, 161, 0, 0, 0, 160, 0, 0, 0, 62, 0, 0, 0, 62, 0, 3, 0, 3, 0, 0, 0, 159, 0, 0, 0, 62, 0, 3, 0, 4, 0, 0, 0, 60, 0, 0, 0, 62, 0, 3, 0, 5, 0, 0, 0, 161, 0, 0, 0, 62, 0, 3, 0, 6, 0, 0, 0, 62, 0, 0, 0, 62, 0, 3, 0, 7, 0, 0, 0, 64, 0, 0, 0, 62, 0, 3, 0, 8, 0, 0, 0, 65, 0, 0, 0, 62, 0, 3, 0, 9, 0, 0, 0, 66, 0, 0, 0, 62, 0, 3, 0, 10, 0, 0, 0, 67, 0, 0, 0, 62, 0, 3, 0, 11, 0, 0, 0, 68, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_SHAPE_FRAG[7560] = {3, 2, 35, 7, 0, 5, 1, 0, 0, 0, 40, 0, 58, 1, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 14, 0, 4, 0, 0, 0, 2, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 10, 0, 0, 0, 11, 0, 0, 0, 16, 0, 3, 0, 2, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 11, 0, 0, 0, 1, 0, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 105, 110, 112, 46, 117, 118, 0, 0, 5, 0, 4, 0, 5, 0, 0, 0, 105, 110, 112, 46, 112, 0, 0, 0, 5, 0, 5, 0, 6, 0, 0, 0, 105, 110, 112, 46, 115, 105, 122, 101, 0, 0, 0, 0, 5, 0, 5, 0, 7, 0, 0, 0, 105, 110, 112, 46, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 7, 0, 8, 0, 0, 0, 105, 110, 112, 46, 98, 111, 114, 100, 101, 114, 95, 99, 111, 108, 111, 114, 0, 0, 0, 0, 5, 0, 7, 0, 9, 0, 0, 0, 105, 110, 112, 46, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 0, 0, 5, 0, 8, 0, 10, 0, 0, 0, 105, 110, 112, 46, 98, 111, 114, 100, 101, 114, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 0, 0, 0, 5, 0, 5, 0, 11, 0, 0, 0, 105, 110, 112, 46, 115, 104, 97, 112, 101, 0, 0, 0, 5, 0, 5, 0, 12, 0, 0, 0, 113, 117, 97, 100, 95, 99, 111, 108, 111, 114, 0, 0, 5, 0, 5, 0, 13, 0, 0, 0, 115, 116, 97, 114, 116, 95, 97, 110, 103, 108, 101, 0, 5, 0, 5, 0, 14, 0, 0, 0, 101, 110, 100, 95, 97, 110, 103, 108, 101, 0, 0, 0, 5, 0, 5, 0, 15, 0, 0, 0, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 0, 0, 5, 0, 3, 0, 16, 0, 0, 0, 112, 0, 0, 0, 5, 0, 4, 0, 17, 0, 0, 0, 97, 108, 112, 104, 97, 0, 0, 0, 5, 0, 7, 0, 3, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 80, 83, 0, 0, 5, 0, 4, 0, 18, 0, 0, 0, 114, 97, 100, 105, 117, 115, 0, 0, 5, 0, 4, 0, 19, 0, 0, 0, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 3, 0, 2, 0, 0, 0, 80, 83, 0, 0, 71, 0, 4, 0, 4, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 5, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 3, 0, 6, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 7, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 3, 0, 7, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 8, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 3, 0, 8, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 9, 0, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 3, 0, 9, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 10, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 3, 0, 10, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 7, 0, 0, 0, 71, 0, 3, 0, 11, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 20, 0, 0, 0, 33, 0, 3, 0, 21, 0, 0, 0, 20, 0, 0, 0, 20, 0, 2, 0, 22, 0, 0, 0, 22, 0, 3, 0, 23, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 24, 0, 0, 0, 23, 0, 0, 0, 4, 0, 0, 0, 23, 0, 4, 0, 25, 0, 0, 0, 23, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 26, 0, 0, 0, 1, 0, 0, 0, 25, 0, 0, 0, 32, 0, 4, 0, 27, 0, 0, 0, 1, 0, 0, 0, 24, 0, 0, 0, 32, 0, 4, 0, 28, 0, 0, 0, 1, 0, 0, 0, 23, 0, 0, 0, 21, 0, 4, 0, 29, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 30, 0, 0, 0, 1, 0, 0, 0, 29, 0, 0, 0, 43, 0, 4, 0, 29, 0, 0, 0, 31, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 29, 0, 0, 0, 32, 0, 0, 0, 2, 0, 0, 0, 43, 0, 4, 0, 23, 0, 0, 0, 33, 0, 0, 0, 0, 0, 0, 0, 41, 0, 3, 0, 22, 0, 0, 0, 34, 0, 0, 0, 43, 0, 4, 0, 23, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 63, 44, 0, 5, 0, 25, 0, 0, 0, 36, 0, 0, 0, 33, 0, 0, 0, 33, 0, 0, 0, 44, 0, 7, 0, 24, 0, 0, 0, 37, 0, 0, 0, 33, 0, 0, 0, 33, 0, 0, 0, 33, 0, 0, 0, 33, 0, 0, 0, 43, 0, 4, 0, 23, 0, 0, 0, 38, 0, 0, 0, 46, 186, 232, 62, 43, 0, 4, 0, 23, 0, 0, 0, 39, 0, 0, 0, 0, 0, 128, 63, 23, 0, 4, 0, 40, 0, 0, 0, 23, 0, 0, 0, 3, 0, 0, 0, 42, 0, 3, 0, 22, 0, 0, 0, 41, 0, 0, 0, 43, 0, 4, 0, 23, 0, 0, 0, 42, 0, 0, 0, 0, 0, 0, 64, 43, 0, 4, 0, 23, 0, 0, 0, 43, 0, 0, 0, 219, 15, 201, 64, 32, 0, 4, 0, 44, 0, 0, 0, 3, 0, 0, 0, 24, 0, 0, 0, 43, 0, 4, 0, 23, 0, 0, 0, 45, 0, 0, 0, 102, 102, 166, 63, 44, 0, 7, 0, 24, 0, 0, 0, 46, 0, 0, 0, 33, 0, 0, 0, 33, 0, 0, 0, 33, 0, 0, 0, 33, 0, 0, 0, 43, 0, 4, 0, 23, 0, 0, 0, 47, 0, 0, 0, 10, 215, 35, 60, 59, 0, 4, 0, 26, 0, 0, 0, 4, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 26, 0, 0, 0, 5, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 26, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 27, 0, 0, 0, 7, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 27, 0, 0, 0, 8, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 27, 0, 0, 0, 9, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 10, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 30, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 44, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 29, 0, 0, 0, 48, 0, 0, 0, 0, 0, 0, 0, 44, 0, 5, 0, 25, 0, 0, 0, 49, 0, 0, 0, 35, 0, 0, 0, 35, 0, 0, 0, 44, 0, 5, 0, 25, 0, 0, 0, 50, 0, 0, 0, 39, 0, 0, 0, 39, 0, 0, 0, 43, 0, 4, 0, 23, 0, 0, 0, 51, 0, 0, 0, 131, 249, 34, 62, 54, 0, 5, 0, 20, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 21, 0, 0, 0, 248, 0, 2, 0, 52, 0, 0, 0, 247, 0, 3, 0, 53, 0, 0, 0, 0, 0, 0, 0, 251, 0, 3, 0, 48, 0, 0, 0, 54, 0, 0, 0, 248, 0, 2, 0, 54, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 55, 0, 0, 0, 4, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 56, 0, 0, 0, 5, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 57, 0, 0, 0, 6, 0, 0, 0, 61, 0, 4, 0, 24, 0, 0, 0, 58, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 24, 0, 0, 0, 59, 0, 0, 0, 8, 0, 0, 0, 61, 0, 4, 0, 24, 0, 0, 0, 60, 0, 0, 0, 9, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 61, 0, 0, 0, 10, 0, 0, 0, 61, 0, 4, 0, 29, 0, 0, 0, 62, 0, 0, 0, 11, 0, 0, 0, 170, 0, 5, 0, 22, 0, 0, 0, 63, 0, 0, 0, 62, 0, 0, 0, 31, 0, 0, 0, 247, 0, 3, 0, 64, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 63, 0, 0, 0, 65, 0, 0, 0, 66, 0, 0, 0, 248, 0, 2, 0, 66, 0, 0, 0, 170, 0, 5, 0, 22, 0, 0, 0, 67, 0, 0, 0, 62, 0, 0, 0, 32, 0, 0, 0, 247, 0, 3, 0, 68, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 67, 0, 0, 0, 69, 0, 0, 0, 70, 0, 0, 0, 248, 0, 2, 0, 70, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 71, 0, 0, 0, 60, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 72, 0, 0, 0, 60, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 73, 0, 0, 0, 60, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 74, 0, 0, 0, 60, 0, 0, 0, 3, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 75, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 73, 0, 0, 0, 74, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 76, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 72, 0, 0, 0, 75, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 77, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 71, 0, 0, 0, 76, 0, 0, 0, 186, 0, 5, 0, 22, 0, 0, 0, 78, 0, 0, 0, 77, 0, 0, 0, 33, 0, 0, 0, 247, 0, 3, 0, 79, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 78, 0, 0, 0, 80, 0, 0, 0, 81, 0, 0, 0, 248, 0, 2, 0, 81, 0, 0, 0, 186, 0, 5, 0, 22, 0, 0, 0, 82, 0, 0, 0, 61, 0, 0, 0, 33, 0, 0, 0, 249, 0, 2, 0, 79, 0, 0, 0, 248, 0, 2, 0, 80, 0, 0, 0, 249, 0, 2, 0, 79, 0, 0, 0, 248, 0, 2, 0, 79, 0, 0, 0, 245, 0, 7, 0, 22, 0, 0, 0, 83, 0, 0, 0, 82, 0, 0, 0, 81, 0, 0, 0, 34, 0, 0, 0, 80, 0, 0, 0, 247, 0, 3, 0, 84, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 83, 0, 0, 0, 85, 0, 0, 0, 86, 0, 0, 0, 248, 0, 2, 0, 86, 0, 0, 0, 249, 0, 2, 0, 84, 0, 0, 0, 248, 0, 2, 0, 85, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 87, 0, 0, 0, 56, 0, 0, 0, 1, 0, 0, 0, 184, 0, 5, 0, 22, 0, 0, 0, 88, 0, 0, 0, 33, 0, 0, 0, 87, 0, 0, 0, 247, 0, 3, 0, 89, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 88, 0, 0, 0, 90, 0, 0, 0, 91, 0, 0, 0, 248, 0, 2, 0, 91, 0, 0, 0, 79, 0, 7, 0, 25, 0, 0, 0, 92, 0, 0, 0, 60, 0, 0, 0, 60, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 249, 0, 2, 0, 89, 0, 0, 0, 248, 0, 2, 0, 90, 0, 0, 0, 79, 0, 7, 0, 25, 0, 0, 0, 93, 0, 0, 0, 60, 0, 0, 0, 60, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 249, 0, 2, 0, 89, 0, 0, 0, 248, 0, 2, 0, 89, 0, 0, 0, 245, 0, 7, 0, 25, 0, 0, 0, 94, 0, 0, 0, 92, 0, 0, 0, 91, 0, 0, 0, 93, 0, 0, 0, 90, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 95, 0, 0, 0, 56, 0, 0, 0, 0, 0, 0, 0, 184, 0, 5, 0, 22, 0, 0, 0, 96, 0, 0, 0, 33, 0, 0, 0, 95, 0, 0, 0, 247, 0, 3, 0, 97, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 96, 0, 0, 0, 98, 0, 0, 0, 99, 0, 0, 0, 248, 0, 2, 0, 99, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 100, 0, 0, 0, 94, 0, 0, 0, 0, 0, 0, 0, 249, 0, 2, 0, 97, 0, 0, 0, 248, 0, 2, 0, 98, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 101, 0, 0, 0, 94, 0, 0, 0, 1, 0, 0, 0, 249, 0, 2, 0, 97, 0, 0, 0, 248, 0, 2, 0, 97, 0, 0, 0, 245, 0, 7, 0, 23, 0, 0, 0, 102, 0, 0, 0, 100, 0, 0, 0, 99, 0, 0, 0, 101, 0, 0, 0, 98, 0, 0, 0, 12, 0, 6, 0, 25, 0, 0, 0, 103, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 56, 0, 0, 0, 142, 0, 5, 0, 25, 0, 0, 0, 104, 0, 0, 0, 57, 0, 0, 0, 35, 0, 0, 0, 131, 0, 5, 0, 25, 0, 0, 0, 105, 0, 0, 0, 103, 0, 0, 0, 104, 0, 0, 0, 80, 0, 5, 0, 25, 0, 0, 0, 106, 0, 0, 0, 102, 0, 0, 0, 102, 0, 0, 0, 129, 0, 5, 0, 25, 0, 0, 0, 107, 0, 0, 0, 105, 0, 0, 0, 106, 0, 0, 0, 12, 0, 7, 0, 25, 0, 0, 0, 108, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 107, 0, 0, 0, 36, 0, 0, 0, 12, 0, 6, 0, 23, 0, 0, 0, 109, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 108, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 110, 0, 0, 0, 107, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 111, 0, 0, 0, 107, 0, 0, 0, 1, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 112, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 110, 0, 0, 0, 111, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 113, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 112, 0, 0, 0, 33, 0, 0, 0, 129, 0, 5, 0, 23, 0, 0, 0, 114, 0, 0, 0, 109, 0, 0, 0, 113, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 115, 0, 0, 0, 114, 0, 0, 0, 102, 0, 0, 0, 80, 0, 7, 0, 24, 0, 0, 0, 116, 0, 0, 0, 61, 0, 0, 0, 61, 0, 0, 0, 61, 0, 0, 0, 61, 0, 0, 0, 79, 0, 7, 0, 25, 0, 0, 0, 117, 0, 0, 0, 116, 0, 0, 0, 116, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 131, 0, 5, 0, 25, 0, 0, 0, 118, 0, 0, 0, 57, 0, 0, 0, 117, 0, 0, 0, 79, 0, 7, 0, 25, 0, 0, 0, 119, 0, 0, 0, 116, 0, 0, 0, 116, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 131, 0, 5, 0, 25, 0, 0, 0, 120, 0, 0, 0, 118, 0, 0, 0, 119, 0, 0, 0, 142, 0, 5, 0, 25, 0, 0, 0, 121, 0, 0, 0, 120, 0, 0, 0, 35, 0, 0, 0, 129, 0, 5, 0, 25, 0, 0, 0, 122, 0, 0, 0, 117, 0, 0, 0, 121, 0, 0, 0, 131, 0, 5, 0, 25, 0, 0, 0, 123, 0, 0, 0, 122, 0, 0, 0, 104, 0, 0, 0, 131, 0, 5, 0, 25, 0, 0, 0, 124, 0, 0, 0, 56, 0, 0, 0, 123, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 125, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 61, 0, 0, 0, 61, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 126, 0, 0, 0, 71, 0, 0, 0, 125, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 127, 0, 0, 0, 72, 0, 0, 0, 125, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 128, 0, 0, 0, 73, 0, 0, 0, 125, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 129, 0, 0, 0, 74, 0, 0, 0, 125, 0, 0, 0, 80, 0, 7, 0, 24, 0, 0, 0, 130, 0, 0, 0, 126, 0, 0, 0, 127, 0, 0, 0, 128, 0, 0, 0, 129, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 131, 0, 0, 0, 121, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 132, 0, 0, 0, 121, 0, 0, 0, 1, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 133, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 131, 0, 0, 0, 132, 0, 0, 0, 12, 0, 7, 0, 24, 0, 0, 0, 134, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 130, 0, 0, 0, 37, 0, 0, 0, 80, 0, 7, 0, 24, 0, 0, 0, 135, 0, 0, 0, 133, 0, 0, 0, 133, 0, 0, 0, 133, 0, 0, 0, 133, 0, 0, 0, 12, 0, 7, 0, 24, 0, 0, 0, 136, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 134, 0, 0, 0, 135, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 137, 0, 0, 0, 124, 0, 0, 0, 1, 0, 0, 0, 184, 0, 5, 0, 22, 0, 0, 0, 138, 0, 0, 0, 33, 0, 0, 0, 137, 0, 0, 0, 247, 0, 3, 0, 139, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 138, 0, 0, 0, 140, 0, 0, 0, 141, 0, 0, 0, 248, 0, 2, 0, 141, 0, 0, 0, 79, 0, 7, 0, 25, 0, 0, 0, 142, 0, 0, 0, 136, 0, 0, 0, 136, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 249, 0, 2, 0, 139, 0, 0, 0, 248, 0, 2, 0, 140, 0, 0, 0, 79, 0, 7, 0, 25, 0, 0, 0, 143, 0, 0, 0, 136, 0, 0, 0, 136, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 249, 0, 2, 0, 139, 0, 0, 0, 248, 0, 2, 0, 139, 0, 0, 0, 245, 0, 7, 0, 25, 0, 0, 0, 144, 0, 0, 0, 142, 0, 0, 0, 141, 0, 0, 0, 143, 0, 0, 0, 140, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 145, 0, 0, 0, 124, 0, 0, 0, 0, 0, 0, 0, 184, 0, 5, 0, 22, 0, 0, 0, 146, 0, 0, 0, 33, 0, 0, 0, 145, 0, 0, 0, 247, 0, 3, 0, 147, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 146, 0, 0, 0, 148, 0, 0, 0, 149, 0, 0, 0, 248, 0, 2, 0, 149, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 150, 0, 0, 0, 144, 0, 0, 0, 0, 0, 0, 0, 249, 0, 2, 0, 147, 0, 0, 0, 248, 0, 2, 0, 148, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 151, 0, 0, 0, 144, 0, 0, 0, 1, 0, 0, 0, 249, 0, 2, 0, 147, 0, 0, 0, 248, 0, 2, 0, 147, 0, 0, 0, 245, 0, 7, 0, 23, 0, 0, 0, 152, 0, 0, 0, 150, 0, 0, 0, 149, 0, 0, 0, 151, 0, 0, 0, 148, 0, 0, 0, 12, 0, 6, 0, 25, 0, 0, 0, 153, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 124, 0, 0, 0, 131, 0, 5, 0, 25, 0, 0, 0, 154, 0, 0, 0, 153, 0, 0, 0, 121, 0, 0, 0, 80, 0, 5, 0, 25, 0, 0, 0, 155, 0, 0, 0, 152, 0, 0, 0, 152, 0, 0, 0, 129, 0, 5, 0, 25, 0, 0, 0, 156, 0, 0, 0, 154, 0, 0, 0, 155, 0, 0, 0, 12, 0, 7, 0, 25, 0, 0, 0, 157, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 156, 0, 0, 0, 36, 0, 0, 0, 12, 0, 6, 0, 23, 0, 0, 0, 158, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 157, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 159, 0, 0, 0, 156, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 160, 0, 0, 0, 156, 0, 0, 0, 1, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 161, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 159, 0, 0, 0, 160, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 162, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 161, 0, 0, 0, 33, 0, 0, 0, 129, 0, 5, 0, 23, 0, 0, 0, 163, 0, 0, 0, 158, 0, 0, 0, 162, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 164, 0, 0, 0, 163, 0, 0, 0, 152, 0, 0, 0, 127, 0, 4, 0, 23, 0, 0, 0, 165, 0, 0, 0, 164, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 166, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 115, 0, 0, 0, 165, 0, 0, 0, 209, 0, 4, 0, 23, 0, 0, 0, 167, 0, 0, 0, 166, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 168, 0, 0, 0, 1, 0, 0, 0, 26, 0, 0, 0, 167, 0, 0, 0, 38, 0, 0, 0, 12, 0, 8, 0, 23, 0, 0, 0, 169, 0, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 33, 0, 0, 0, 168, 0, 0, 0, 166, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 170, 0, 0, 0, 39, 0, 0, 0, 169, 0, 0, 0, 209, 0, 4, 0, 23, 0, 0, 0, 171, 0, 0, 0, 115, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 172, 0, 0, 0, 1, 0, 0, 0, 26, 0, 0, 0, 171, 0, 0, 0, 38, 0, 0, 0, 12, 0, 8, 0, 23, 0, 0, 0, 173, 0, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 33, 0, 0, 0, 172, 0, 0, 0, 115, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 174, 0, 0, 0, 39, 0, 0, 0, 173, 0, 0, 0, 79, 0, 8, 0, 40, 0, 0, 0, 175, 0, 0, 0, 58, 0, 0, 0, 58, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 176, 0, 0, 0, 58, 0, 0, 0, 3, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 177, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 176, 0, 0, 0, 174, 0, 0, 0, 80, 0, 5, 0, 24, 0, 0, 0, 12, 0, 0, 0, 175, 0, 0, 0, 177, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 178, 0, 0, 0, 59, 0, 0, 0, 3, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 179, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 170, 0, 0, 0, 174, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 180, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 178, 0, 0, 0, 179, 0, 0, 0, 80, 0, 7, 0, 24, 0, 0, 0, 181, 0, 0, 0, 180, 0, 0, 0, 180, 0, 0, 0, 180, 0, 0, 0, 180, 0, 0, 0, 12, 0, 8, 0, 24, 0, 0, 0, 182, 0, 0, 0, 1, 0, 0, 0, 46, 0, 0, 0, 12, 0, 0, 0, 59, 0, 0, 0, 181, 0, 0, 0, 186, 0, 5, 0, 22, 0, 0, 0, 183, 0, 0, 0, 164, 0, 0, 0, 33, 0, 0, 0, 247, 0, 3, 0, 184, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 183, 0, 0, 0, 185, 0, 0, 0, 186, 0, 0, 0, 248, 0, 2, 0, 186, 0, 0, 0, 249, 0, 2, 0, 184, 0, 0, 0, 248, 0, 2, 0, 185, 0, 0, 0, 184, 0, 5, 0, 22, 0, 0, 0, 187, 0, 0, 0, 170, 0, 0, 0, 39, 0, 0, 0, 249, 0, 2, 0, 184, 0, 0, 0, 248, 0, 2, 0, 184, 0, 0, 0, 245, 0, 7, 0, 22, 0, 0, 0, 188, 0, 0, 0, 41, 0, 0, 0, 186, 0, 0, 0, 187, 0, 0, 0, 185, 0, 0, 0, 247, 0, 3, 0, 189, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 188, 0, 0, 0, 190, 0, 0, 0, 191, 0, 0, 0, 248, 0, 2, 0, 191, 0, 0, 0, 249, 0, 2, 0, 189, 0, 0, 0, 248, 0, 2, 0, 190, 0, 0, 0, 79, 0, 8, 0, 40, 0, 0, 0, 192, 0, 0, 0, 59, 0, 0, 0, 59, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 80, 0, 5, 0, 24, 0, 0, 0, 193, 0, 0, 0, 192, 0, 0, 0, 170, 0, 0, 0, 249, 0, 2, 0, 189, 0, 0, 0, 248, 0, 2, 0, 189, 0, 0, 0, 245, 0, 7, 0, 24, 0, 0, 0, 194, 0, 0, 0, 182, 0, 0, 0, 191, 0, 0, 0, 193, 0, 0, 0, 190, 0, 0, 0, 249, 0, 2, 0, 84, 0, 0, 0, 248, 0, 2, 0, 84, 0, 0, 0, 245, 0, 7, 0, 24, 0, 0, 0, 195, 0, 0, 0, 58, 0, 0, 0, 86, 0, 0, 0, 194, 0, 0, 0, 189, 0, 0, 0, 249, 0, 2, 0, 68, 0, 0, 0, 248, 0, 2, 0, 68, 0, 0, 0, 249, 0, 2, 0, 64, 0, 0, 0, 248, 0, 2, 0, 69, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 13, 0, 0, 0, 60, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 14, 0, 0, 0, 60, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 196, 0, 0, 0, 57, 0, 0, 0, 1, 0, 0, 0, 136, 0, 5, 0, 23, 0, 0, 0, 197, 0, 0, 0, 61, 0, 0, 0, 196, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 15, 0, 0, 0, 35, 0, 0, 0, 197, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 198, 0, 0, 0, 55, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 199, 0, 0, 0, 55, 0, 0, 0, 1, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 200, 0, 0, 0, 39, 0, 0, 0, 199, 0, 0, 0, 80, 0, 5, 0, 25, 0, 0, 0, 201, 0, 0, 0, 198, 0, 0, 0, 200, 0, 0, 0, 142, 0, 5, 0, 25, 0, 0, 0, 202, 0, 0, 0, 201, 0, 0, 0, 42, 0, 0, 0, 131, 0, 5, 0, 25, 0, 0, 0, 203, 0, 0, 0, 202, 0, 0, 0, 50, 0, 0, 0, 133, 0, 5, 0, 25, 0, 0, 0, 204, 0, 0, 0, 203, 0, 0, 0, 57, 0, 0, 0, 80, 0, 5, 0, 25, 0, 0, 0, 205, 0, 0, 0, 196, 0, 0, 0, 196, 0, 0, 0, 136, 0, 5, 0, 25, 0, 0, 0, 16, 0, 0, 0, 204, 0, 0, 0, 205, 0, 0, 0, 129, 0, 5, 0, 23, 0, 0, 0, 206, 0, 0, 0, 197, 0, 0, 0, 35, 0, 0, 0, 247, 0, 3, 0, 207, 0, 0, 0, 0, 0, 0, 0, 251, 0, 3, 0, 48, 0, 0, 0, 208, 0, 0, 0, 248, 0, 2, 0, 208, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 209, 0, 0, 0, 16, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 210, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 211, 0, 0, 0, 1, 0, 0, 0, 25, 0, 0, 0, 209, 0, 0, 0, 210, 0, 0, 0, 133, 0, 5, 0, 23, 0, 0, 0, 212, 0, 0, 0, 211, 0, 0, 0, 51, 0, 0, 0, 12, 0, 6, 0, 23, 0, 0, 0, 213, 0, 0, 0, 1, 0, 0, 0, 8, 0, 0, 0, 212, 0, 0, 0, 133, 0, 5, 0, 23, 0, 0, 0, 214, 0, 0, 0, 43, 0, 0, 0, 213, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 215, 0, 0, 0, 211, 0, 0, 0, 214, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 216, 0, 0, 0, 215, 0, 0, 0, 13, 0, 0, 0, 184, 0, 5, 0, 22, 0, 0, 0, 217, 0, 0, 0, 216, 0, 0, 0, 33, 0, 0, 0, 247, 0, 3, 0, 218, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 217, 0, 0, 0, 219, 0, 0, 0, 220, 0, 0, 0, 248, 0, 2, 0, 220, 0, 0, 0, 249, 0, 2, 0, 218, 0, 0, 0, 248, 0, 2, 0, 219, 0, 0, 0, 129, 0, 5, 0, 23, 0, 0, 0, 221, 0, 0, 0, 216, 0, 0, 0, 43, 0, 0, 0, 249, 0, 2, 0, 218, 0, 0, 0, 248, 0, 2, 0, 218, 0, 0, 0, 245, 0, 7, 0, 23, 0, 0, 0, 222, 0, 0, 0, 216, 0, 0, 0, 220, 0, 0, 0, 221, 0, 0, 0, 219, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 223, 0, 0, 0, 14, 0, 0, 0, 13, 0, 0, 0, 184, 0, 5, 0, 22, 0, 0, 0, 224, 0, 0, 0, 223, 0, 0, 0, 33, 0, 0, 0, 247, 0, 3, 0, 225, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 224, 0, 0, 0, 226, 0, 0, 0, 227, 0, 0, 0, 248, 0, 2, 0, 227, 0, 0, 0, 249, 0, 2, 0, 225, 0, 0, 0, 248, 0, 2, 0, 226, 0, 0, 0, 129, 0, 5, 0, 23, 0, 0, 0, 228, 0, 0, 0, 223, 0, 0, 0, 43, 0, 0, 0, 249, 0, 2, 0, 225, 0, 0, 0, 248, 0, 2, 0, 225, 0, 0, 0, 245, 0, 7, 0, 23, 0, 0, 0, 229, 0, 0, 0, 223, 0, 0, 0, 227, 0, 0, 0, 228, 0, 0, 0, 226, 0, 0, 0, 190, 0, 5, 0, 22, 0, 0, 0, 230, 0, 0, 0, 222, 0, 0, 0, 229, 0, 0, 0, 247, 0, 3, 0, 231, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 230, 0, 0, 0, 232, 0, 0, 0, 231, 0, 0, 0, 248, 0, 2, 0, 231, 0, 0, 0, 12, 0, 6, 0, 23, 0, 0, 0, 233, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 16, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 234, 0, 0, 0, 233, 0, 0, 0, 206, 0, 0, 0, 12, 0, 6, 0, 23, 0, 0, 0, 235, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 234, 0, 0, 0, 249, 0, 2, 0, 207, 0, 0, 0, 248, 0, 2, 0, 232, 0, 0, 0, 12, 0, 6, 0, 23, 0, 0, 0, 236, 0, 0, 0, 1, 0, 0, 0, 14, 0, 0, 0, 13, 0, 0, 0, 133, 0, 5, 0, 23, 0, 0, 0, 237, 0, 0, 0, 206, 0, 0, 0, 236, 0, 0, 0, 12, 0, 6, 0, 23, 0, 0, 0, 238, 0, 0, 0, 1, 0, 0, 0, 13, 0, 0, 0, 13, 0, 0, 0, 133, 0, 5, 0, 23, 0, 0, 0, 239, 0, 0, 0, 206, 0, 0, 0, 238, 0, 0, 0, 80, 0, 5, 0, 25, 0, 0, 0, 240, 0, 0, 0, 237, 0, 0, 0, 239, 0, 0, 0, 12, 0, 6, 0, 23, 0, 0, 0, 241, 0, 0, 0, 1, 0, 0, 0, 14, 0, 0, 0, 14, 0, 0, 0, 133, 0, 5, 0, 23, 0, 0, 0, 242, 0, 0, 0, 206, 0, 0, 0, 241, 0, 0, 0, 12, 0, 6, 0, 23, 0, 0, 0, 243, 0, 0, 0, 1, 0, 0, 0, 13, 0, 0, 0, 14, 0, 0, 0, 133, 0, 5, 0, 23, 0, 0, 0, 244, 0, 0, 0, 206, 0, 0, 0, 243, 0, 0, 0, 80, 0, 5, 0, 25, 0, 0, 0, 245, 0, 0, 0, 242, 0, 0, 0, 244, 0, 0, 0, 131, 0, 5, 0, 25, 0, 0, 0, 246, 0, 0, 0, 16, 0, 0, 0, 240, 0, 0, 0, 12, 0, 6, 0, 23, 0, 0, 0, 247, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 246, 0, 0, 0, 131, 0, 5, 0, 25, 0, 0, 0, 248, 0, 0, 0, 16, 0, 0, 0, 245, 0, 0, 0, 12, 0, 6, 0, 23, 0, 0, 0, 249, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 248, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 250, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 247, 0, 0, 0, 249, 0, 0, 0, 249, 0, 2, 0, 207, 0, 0, 0, 248, 0, 2, 0, 207, 0, 0, 0, 245, 0, 7, 0, 23, 0, 0, 0, 251, 0, 0, 0, 235, 0, 0, 0, 231, 0, 0, 0, 250, 0, 0, 0, 232, 0, 0, 0, 207, 0, 4, 0, 23, 0, 0, 0, 252, 0, 0, 0, 251, 0, 0, 0, 208, 0, 4, 0, 23, 0, 0, 0, 253, 0, 0, 0, 251, 0, 0, 0, 80, 0, 5, 0, 25, 0, 0, 0, 254, 0, 0, 0, 252, 0, 0, 0, 253, 0, 0, 0, 12, 0, 6, 0, 23, 0, 0, 0, 255, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 254, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 0, 1, 0, 0, 15, 0, 0, 0, 255, 0, 0, 0, 12, 0, 8, 0, 23, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 0, 1, 0, 0, 15, 0, 0, 0, 251, 0, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 17, 0, 0, 0, 39, 0, 0, 0, 1, 1, 0, 0, 79, 0, 8, 0, 40, 0, 0, 0, 2, 1, 0, 0, 58, 0, 0, 0, 58, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 3, 1, 0, 0, 58, 0, 0, 0, 3, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 4, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 3, 1, 0, 0, 17, 0, 0, 0, 80, 0, 5, 0, 24, 0, 0, 0, 5, 1, 0, 0, 2, 1, 0, 0, 4, 1, 0, 0, 62, 0, 3, 0, 3, 0, 0, 0, 5, 1, 0, 0, 249, 0, 2, 0, 53, 0, 0, 0, 248, 0, 2, 0, 65, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 6, 1, 0, 0, 57, 0, 0, 0, 0, 0, 0, 0, 136, 0, 5, 0, 23, 0, 0, 0, 7, 1, 0, 0, 39, 0, 0, 0, 6, 1, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 18, 0, 0, 0, 35, 0, 0, 0, 7, 1, 0, 0, 131, 0, 5, 0, 25, 0, 0, 0, 8, 1, 0, 0, 55, 0, 0, 0, 49, 0, 0, 0, 12, 0, 6, 0, 23, 0, 0, 0, 9, 1, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 8, 1, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 10, 1, 0, 0, 9, 1, 0, 0, 18, 0, 0, 0, 209, 0, 4, 0, 23, 0, 0, 0, 11, 1, 0, 0, 10, 1, 0, 0, 133, 0, 5, 0, 23, 0, 0, 0, 12, 1, 0, 0, 11, 1, 0, 0, 45, 0, 0, 0, 12, 0, 8, 0, 23, 0, 0, 0, 13, 1, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 33, 0, 0, 0, 12, 1, 0, 0, 10, 1, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 14, 1, 0, 0, 39, 0, 0, 0, 13, 1, 0, 0, 186, 0, 5, 0, 22, 0, 0, 0, 15, 1, 0, 0, 61, 0, 0, 0, 33, 0, 0, 0, 247, 0, 3, 0, 16, 1, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 15, 1, 0, 0, 17, 1, 0, 0, 18, 1, 0, 0, 248, 0, 2, 0, 18, 1, 0, 0, 79, 0, 8, 0, 40, 0, 0, 0, 19, 1, 0, 0, 58, 0, 0, 0, 58, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 20, 1, 0, 0, 58, 0, 0, 0, 3, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 21, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 20, 1, 0, 0, 14, 1, 0, 0, 80, 0, 5, 0, 24, 0, 0, 0, 22, 1, 0, 0, 19, 1, 0, 0, 21, 1, 0, 0, 249, 0, 2, 0, 16, 1, 0, 0, 248, 0, 2, 0, 17, 1, 0, 0, 136, 0, 5, 0, 23, 0, 0, 0, 23, 1, 0, 0, 61, 0, 0, 0, 6, 1, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 24, 1, 0, 0, 18, 0, 0, 0, 23, 1, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 25, 1, 0, 0, 9, 1, 0, 0, 24, 1, 0, 0, 127, 0, 4, 0, 23, 0, 0, 0, 26, 1, 0, 0, 25, 1, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 27, 1, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 10, 1, 0, 0, 26, 1, 0, 0, 209, 0, 4, 0, 23, 0, 0, 0, 28, 1, 0, 0, 27, 1, 0, 0, 133, 0, 5, 0, 23, 0, 0, 0, 29, 1, 0, 0, 28, 1, 0, 0, 45, 0, 0, 0, 12, 0, 8, 0, 23, 0, 0, 0, 30, 1, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 33, 0, 0, 0, 29, 1, 0, 0, 27, 1, 0, 0, 131, 0, 5, 0, 23, 0, 0, 0, 31, 1, 0, 0, 39, 0, 0, 0, 30, 1, 0, 0, 79, 0, 8, 0, 40, 0, 0, 0, 32, 1, 0, 0, 58, 0, 0, 0, 58, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 33, 1, 0, 0, 58, 0, 0, 0, 3, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 34, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 33, 1, 0, 0, 14, 1, 0, 0, 80, 0, 5, 0, 24, 0, 0, 0, 19, 0, 0, 0, 32, 1, 0, 0, 34, 1, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 35, 1, 0, 0, 59, 0, 0, 0, 3, 0, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 36, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 31, 1, 0, 0, 14, 1, 0, 0, 12, 0, 7, 0, 23, 0, 0, 0, 37, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 35, 1, 0, 0, 36, 1, 0, 0, 80, 0, 7, 0, 24, 0, 0, 0, 38, 1, 0, 0, 37, 1, 0, 0, 37, 1, 0, 0, 37, 1, 0, 0, 37, 1, 0, 0, 12, 0, 8, 0, 24, 0, 0, 0, 39, 1, 0, 0, 1, 0, 0, 0, 46, 0, 0, 0, 19, 0, 0, 0, 59, 0, 0, 0, 38, 1, 0, 0, 186, 0, 5, 0, 22, 0, 0, 0, 40, 1, 0, 0, 25, 1, 0, 0, 33, 0, 0, 0, 247, 0, 3, 0, 41, 1, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 40, 1, 0, 0, 42, 1, 0, 0, 43, 1, 0, 0, 248, 0, 2, 0, 43, 1, 0, 0, 249, 0, 2, 0, 41, 1, 0, 0, 248, 0, 2, 0, 42, 1, 0, 0, 184, 0, 5, 0, 22, 0, 0, 0, 44, 1, 0, 0, 31, 1, 0, 0, 39, 0, 0, 0, 249, 0, 2, 0, 41, 1, 0, 0, 248, 0, 2, 0, 41, 1, 0, 0, 245, 0, 7, 0, 22, 0, 0, 0, 45, 1, 0, 0, 41, 0, 0, 0, 43, 1, 0, 0, 44, 1, 0, 0, 42, 1, 0, 0, 247, 0, 3, 0, 46, 1, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 45, 1, 0, 0, 47, 1, 0, 0, 48, 1, 0, 0, 248, 0, 2, 0, 48, 1, 0, 0, 249, 0, 2, 0, 46, 1, 0, 0, 248, 0, 2, 0, 47, 1, 0, 0, 80, 0, 7, 0, 24, 0, 0, 0, 49, 1, 0, 0, 31, 1, 0, 0, 31, 1, 0, 0, 31, 1, 0, 0, 31, 1, 0, 0, 12, 0, 8, 0, 24, 0, 0, 0, 50, 1, 0, 0, 1, 0, 0, 0, 46, 0, 0, 0, 46, 0, 0, 0, 59, 0, 0, 0, 49, 1, 0, 0, 249, 0, 2, 0, 46, 1, 0, 0, 248, 0, 2, 0, 46, 1, 0, 0, 245, 0, 7, 0, 24, 0, 0, 0, 51, 1, 0, 0, 39, 1, 0, 0, 48, 1, 0, 0, 50, 1, 0, 0, 47, 1, 0, 0, 249, 0, 2, 0, 16, 1, 0, 0, 248, 0, 2, 0, 16, 1, 0, 0, 245, 0, 7, 0, 24, 0, 0, 0, 52, 1, 0, 0, 22, 1, 0, 0, 18, 1, 0, 0, 51, 1, 0, 0, 46, 1, 0, 0, 249, 0, 2, 0, 64, 0, 0, 0, 248, 0, 2, 0, 64, 0, 0, 0, 245, 0, 7, 0, 24, 0, 0, 0, 53, 1, 0, 0, 195, 0, 0, 0, 68, 0, 0, 0, 52, 1, 0, 0, 16, 1, 0, 0, 81, 0, 5, 0, 23, 0, 0, 0, 54, 1, 0, 0, 53, 1, 0, 0, 3, 0, 0, 0, 188, 0, 5, 0, 22, 0, 0, 0, 55, 1, 0, 0, 54, 1, 0, 0, 47, 0, 0, 0, 247, 0, 3, 0, 56, 1, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 55, 1, 0, 0, 57, 1, 0, 0, 56, 1, 0, 0, 248, 0, 2, 0, 57, 1, 0, 0, 252, 0, 1, 0, 248, 0, 2, 0, 56, 1, 0, 0, 62, 0, 3, 0, 3, 0, 0, 0, 53, 1, 0, 0, 249, 0, 2, 0, 53, 0, 0, 0, 248, 0, 2, 0, 53, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const char METAL_SHAPE_VERT[7884] = R"(#include <metal_stdlib>
#include <metal_math>
#include <metal_texture>
using namespace metal;
struct VS_Result_0
{
float4 position_0 [[position]];
float2 uv_0 [[user(UV)]];
float2 p_0 [[user(POINT)]];
float2 size_0 [[user(SIZE)]];
float4 color_0 [[user(COLOR)]];
float4 border_color_0 [[user(BORDERCOLOR)]];
float4 border_radius_0 [[user(BORDERRADIUS)]];
float border_thickness_0 [[user(BORDERTHICKNESS)]];
uint shape_0 [[user(SHAPE)]];
};
struct vertexInput_0
{
float2 position_1 [[attribute(0)]];
float3 i_position_0 [[attribute(1)]];
float2 i_size_0 [[attribute(2)]];
float2 i_offset_0 [[attribute(3)]];
float4 i_color_0 [[attribute(4)]];
float4 i_border_color_0 [[attribute(5)]];
float4 i_border_radius_0 [[attribute(6)]];
float i_border_thickness_0 [[attribute(7)]];
uint i_shape_0 [[attribute(8)]];
uint i_flags_0 [[attribute(9)]];
};
struct _MatrixStorage_float4x4_ColMajornatural_0
{
array<float4, int(4)> data_0;
};
struct GlobalUniforms_natural_0
{
_MatrixStorage_float4x4_ColMajornatural_0 screen_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 view_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 nonscale_view_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 nonscale_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 transform_matrix_0;
_MatrixStorage_float4x4_ColMajornatural_0 inv_view_proj_0;
float2 camera_position_0;
float2 window_size_0;
};
struct SLANG_ParameterGroup_GlobalUniformBuffer_natural_0
{
GlobalUniforms_natural_0 uniforms_0;
};
struct KernelContext_0
{
SLANG_ParameterGroup_GlobalUniformBuffer_natural_0 constant* GlobalUniformBuffer_0;
};
struct VSOutput_0
{
float4 position_2;
float2 uv_1;
float2 p_1;
[[flat]] float2 size_1;
[[flat]] float4 color_1;
[[flat]] float4 border_color_1;
[[flat]] float4 border_radius_1;
[[flat]] float border_thickness_1;
[[flat]] uint shape_1;
};
[[vertex]] VS_Result_0 VS(vertexInput_0 _S1 [[stage_in]], SLANG_ParameterGroup_GlobalUniformBuffer_natural_0 constant* GlobalUniformBuffer_1 [[buffer(2)]])
{
KernelContext_0 kernelContext_0;
(&kernelContext_0)->GlobalUniformBuffer_0 = GlobalUniformBuffer_1;
thread matrix<float,int(4),int(4)>  transform_0 = matrix<float,int(4),int(4)> (float4(1.0, 0.0, 0.0, _S1.i_position_0.x), float4(0.0, 1.0, 0.0, _S1.i_position_0.y), float4(0.0, 0.0, 1.0, 0.0), float4(0.0, 0.0, 0.0, 1.0));
float2 offset_0 = - _S1.i_offset_0 * _S1.i_size_0;
transform_0[int(0)].w = transform_0[int(0)].x * offset_0[int(0)] + transform_0[int(0)].y * offset_0[int(1)] + transform_0[int(0)].z * 0.0 + transform_0[int(0)].w;
transform_0[int(1)].w = transform_0[int(1)].x * offset_0[int(0)] + transform_0[int(1)].y * offset_0[int(1)] + transform_0[int(1)].z * 0.0 + transform_0[int(1)].w;
transform_0[int(2)].w = transform_0[int(2)].x * offset_0[int(0)] + transform_0[int(2)].y * offset_0[int(1)] + transform_0[int(2)].z * 0.0 + transform_0[int(2)].w;
transform_0[int(3)].w = transform_0[int(3)].x * offset_0[int(0)] + transform_0[int(3)].y * offset_0[int(1)] + transform_0[int(3)].z * 0.0 + transform_0[int(3)].w;
transform_0[int(0)].x = transform_0[int(0)].x * _S1.i_size_0[int(0)];
transform_0[int(1)].x = transform_0[int(1)].x * _S1.i_size_0[int(0)];
transform_0[int(2)].x = transform_0[int(2)].x * _S1.i_size_0[int(0)];
transform_0[int(3)].x = transform_0[int(3)].x * _S1.i_size_0[int(0)];
transform_0[int(0)].y = transform_0[int(0)].y * _S1.i_size_0[int(1)];
transform_0[int(1)].y = transform_0[int(1)].y * _S1.i_size_0[int(1)];
transform_0[int(2)].y = transform_0[int(2)].y * _S1.i_size_0[int(1)];
transform_0[int(3)].y = transform_0[int(3)].y * _S1.i_size_0[int(1)];
matrix<float,int(4),int(4)>  _S2;
if(((_S1.i_flags_0) & 1U) == 1U)
{
_S2 = matrix<float,int(4),int(4)> ((&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(3)]);
}
else
{
_S2 = matrix<float,int(4),int(4)> ((&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(3)]);
}
thread VSOutput_0 outp_0;
(&outp_0)->position_2 = (((float4(_S1.position_1, 0.0, 1.0)) * ((((transform_0) * (_S2))))));
(&outp_0)->position_2.z = _S1.i_position_0.z;
(&outp_0)->uv_1 = _S1.position_1;
(&outp_0)->p_1 = (_S1.position_1 - float2(0.5) ) * _S1.i_size_0;
(&outp_0)->size_1 = _S1.i_size_0;
(&outp_0)->color_1 = _S1.i_color_0;
(&outp_0)->border_color_1 = _S1.i_border_color_0;
(&outp_0)->border_thickness_1 = _S1.i_border_thickness_0;
(&outp_0)->border_radius_1 = _S1.i_border_radius_0;
(&outp_0)->shape_1 = _S1.i_shape_0;
VSOutput_0 _S3 = outp_0;
thread VS_Result_0 _S4;
(&_S4)->position_0 = _S3.position_2;
(&_S4)->uv_0 = _S3.uv_1;
(&_S4)->p_0 = _S3.p_1;
(&_S4)->size_0 = _S3.size_1;
(&_S4)->color_0 = _S3.color_1;
(&_S4)->border_color_0 = _S3.border_color_1;
(&_S4)->border_radius_0 = _S3.border_radius_1;
(&_S4)->border_thickness_0 = _S3.border_thickness_1;
(&_S4)->shape_0 = _S3.shape_1;
return _S4;
}
)";

static const char METAL_SHAPE_FRAG[5813] = R"(#include <metal_stdlib>
#include <metal_math>
#include <metal_texture>
using namespace metal;
float sdf_circle_0(const float2 thread* p_0, float radius_0)
{
return length(*p_0) - radius_0;
}
float antialias_circle_0(float d_0)
{
return 1.0 - smoothstep(0.0, (fwidth((d_0))) * 1.29999995231628418, d_0);
}
float mod_0(float x_0, float y_0)
{
return x_0 - y_0 * floor(x_0 / y_0);
}
float arc_sdf_0(const float2 thread* p_1, float a0_0, float a1_0, float r_0)
{
float ap_0 = mod_0(atan2((*p_1).y, (*p_1).x), 6.28318548202514648) - a0_0;
float ap_1;
if(ap_0 < 0.0)
{
ap_1 = ap_0 + 6.28318548202514648;
}
else
{
ap_1 = ap_0;
}
float a1p_0 = a1_0 - a0_0;
float a1p_1;
if(a1p_0 < 0.0)
{
a1p_1 = a1p_0 + 6.28318548202514648;
}
else
{
a1p_1 = a1p_0;
}
if(ap_1 >= a1p_1)
{
return min(length(*p_1 - float2(r_0 * cos(a0_0), r_0 * sin(a0_0))), length(*p_1 - float2(r_0 * cos(a1_0), r_0 * sin(a1_0))));
}
return abs(length(*p_1) - r_0);
}
float sd_rounded_box_0(const float2 thread* p_2, const float2 thread* size_0, const float4 thread* corner_radii_0)
{
float2 rs_0;
if(0.0 < ((*p_2).y))
{
rs_0 = (*corner_radii_0).zw;
}
else
{
rs_0 = (*corner_radii_0).xy;
}
float radius_1;
if(0.0 < ((*p_2).x))
{
radius_1 = rs_0.y;
}
else
{
radius_1 = rs_0.x;
}
float2 q_0 = abs(*p_2) - float2(0.5)  * *size_0 + float2(radius_1) ;
return length(max(q_0, float2(0.0, 0.0))) + min(max(q_0.x, q_0.y), 0.0) - radius_1;
}
float sd_inset_rounded_box_0(const float2 thread* p_3, const float2 thread* size_1, const float4 thread* radius_2, const float4 thread* inset_0)
{
float2 _S1 = (*inset_0).xy;
float2 inner_size_0 = *size_1 - _S1 - (*inset_0).zw;
float2 _S2 = float2(0.5) ;
float2 inner_point_0 = *p_3 - (_S1 + _S2 * inner_size_0 - _S2 * *size_1);
thread float4 r_1 = *radius_2;
float _S3 = (*inset_0).x;
float _S4 = (*inset_0).y;
r_1.x = (*radius_2).x - max(_S3, _S4);
float _S5 = (*inset_0).z;
r_1.y = r_1.y - max(_S5, _S4);
float _S6 = (*inset_0).w;
r_1.z = r_1.z - max(_S5, _S6);
r_1.w = r_1.w - max(_S3, _S6);
float2 half_size_0 = inner_size_0 * _S2;
float min_size_0 = min(half_size_0.x, half_size_0.y);
float4 _S7 = min(max(r_1, float4(0.0, 0.0, 0.0, 0.0)), float4(min_size_0, min_size_0, min_size_0, min_size_0));
r_1 = _S7;
float2 _S8 = inner_point_0;
float2 _S9 = inner_size_0;
float4 _S10 = _S7;
float _S11 = sd_rounded_box_0(&_S8, &_S9, &_S10);
return _S11;
}
float antialias_0(float d_1)
{
return 1.0 - smoothstep(0.0, pow((fwidth((d_1))), 0.45454543828964233), d_1);
}
struct pixelOutput_0
{
float4 output_0 [[color(0)]];
};
struct pixelInput_0
{
float2 uv_0 [[user(UV)]];
float2 p_4 [[user(POINT)]];
[[flat]] float2 size_2 [[user(SIZE)]];
[[flat]] float4 color_0 [[user(COLOR)]];
[[flat]] float4 border_color_0 [[user(BORDERCOLOR)]];
[[flat]] float4 border_radius_0 [[user(BORDERRADIUS)]];
[[flat]] float border_thickness_0 [[user(BORDERTHICKNESS)]];
[[flat]] uint shape_0 [[user(SHAPE)]];
};
[[fragment]] pixelOutput_0 PS(pixelInput_0 _S12 [[stage_in]], float4 position_0 [[position]])
{
bool _S13;
float4 result_0;
if((_S12.shape_0) == 1U)
{
float _S14 = _S12.size_2.x;
float radius_3 = 0.5 - 1.0 / _S14;
float2 _S15 = _S12.uv_0 - float2(0.5) ;
float2 _S16 = _S15;
float _S17 = sdf_circle_0(&_S16, radius_3);
float alpha_0 = antialias_circle_0(_S17);
if((_S12.border_thickness_0) > 0.0)
{
float _S18 = radius_3 - _S12.border_thickness_0 / _S14;
float2 _S19 = _S15;
float _S20 = sdf_circle_0(&_S19, _S18);
float border_alpha_0 = antialias_circle_0(max(_S17, - _S20));
float4 color_with_border_0 = mix(float4(_S12.color_0.xyz, min(_S12.color_0.w, alpha_0)), _S12.border_color_0, float4(min(_S12.border_color_0.w, min(border_alpha_0, alpha_0))) );
if(_S20 > 0.0)
{
_S13 = border_alpha_0 < 1.0;
}
else
{
_S13 = false;
}
if(_S13)
{
result_0 = mix(float4(0.0, 0.0, 0.0, 0.0), _S12.border_color_0, float4(border_alpha_0) );
}
else
{
result_0 = color_with_border_0;
}
}
else
{
result_0 = float4(_S12.color_0.xyz, min(_S12.color_0.w, alpha_0));
}
}
else
{
if((_S12.shape_0) == 2U)
{
float start_angle_0 = _S12.border_radius_0.x;
float end_angle_0 = _S12.border_radius_0.y;
float _S21 = _S12.size_2.y;
float thickness_0 = 0.5 - _S12.border_thickness_0 / _S21;
float _S22 = 1.0 - thickness_0;
float2 _S23 = (float2(_S12.uv_0.x, 1.0 - _S12.uv_0.y) * float2(2.0)  - float2(1.0) ) * _S12.size_2 / float2(_S21) ;
float _S24 = arc_sdf_0(&_S23, start_angle_0, end_angle_0, _S22);
pixelOutput_0 _S25 = { float4(_S12.color_0.xyz, min(_S12.color_0.w, 1.0 - smoothstep(thickness_0 - length(float2(dfdx(_S24), dfdy(_S24))), thickness_0, _S24))) };
return _S25;
}
else
{
if((max(_S12.border_radius_0.x, max(_S12.border_radius_0.y, max(_S12.border_radius_0.z, _S12.border_radius_0.w)))) > 0.0)
{
_S13 = true;
}
else
{
_S13 = (_S12.border_thickness_0) > 0.0;
}
if(_S13)
{
float2 _S26 = _S12.p_4;
float2 _S27 = _S12.size_2;
float4 _S28 = _S12.border_radius_0;
float _S29 = sd_rounded_box_0(&_S26, &_S27, &_S28);
float4 _S30 = float4(_S12.border_thickness_0, _S12.border_thickness_0, _S12.border_thickness_0, _S12.border_thickness_0);
float2 _S31 = _S12.p_4;
float2 _S32 = _S12.size_2;
float4 _S33 = _S12.border_radius_0;
float4 _S34 = _S30;
float _S35 = sd_inset_rounded_box_0(&_S31, &_S32, &_S33, &_S34);
float border_alpha_1 = antialias_0(max(_S29, - _S35));
float smoothed_alpha_0 = antialias_0(_S29);
float4 quad_color_with_border_0 = mix(float4(_S12.color_0.xyz, min(_S12.color_0.w, smoothed_alpha_0)), _S12.border_color_0, float4(min(_S12.border_color_0.w, min(border_alpha_1, smoothed_alpha_0))) );
if(_S35 > 0.0)
{
_S13 = border_alpha_1 < 1.0;
}
else
{
_S13 = false;
}
if(_S13)
{
result_0 = float4(_S12.border_color_0.xyz, border_alpha_1);
}
else
{
result_0 = quad_color_with_border_0;
}
}
else
{
result_0 = _S12.color_0;
}
}
}
if((result_0.w) <= 0.00999999977648258)
{
discard_fragment();
}
pixelOutput_0 _S36 = { result_0 };
return _S36;
}
)";

static const char GL_SHAPE_VERT[4471] = R"(#version 410
struct _MatrixStorage_float4x4_ColMajorstd140
{
vec4 data[4];
};
struct GlobalUniforms_std140
{
_MatrixStorage_float4x4_ColMajorstd140 screen_projection;
_MatrixStorage_float4x4_ColMajorstd140 view_projection;
_MatrixStorage_float4x4_ColMajorstd140 nonscale_view_projection;
_MatrixStorage_float4x4_ColMajorstd140 nonscale_projection;
_MatrixStorage_float4x4_ColMajorstd140 transform_matrix;
_MatrixStorage_float4x4_ColMajorstd140 inv_view_proj;
vec2 camera_position;
vec2 window_size;
};
layout(std140) uniform GlobalUniformBuffer_std140
{
GlobalUniforms_std140 uniforms;
} GlobalUniformBuffer;
layout(location = 0) in vec2 inp_position;
layout(location = 1) in vec3 inp_i_position;
layout(location = 2) in vec2 inp_i_size;
layout(location = 3) in vec2 inp_i_offset;
layout(location = 4) in vec4 inp_i_color;
layout(location = 5) in vec4 inp_i_border_color;
layout(location = 6) in vec4 inp_i_border_radius;
layout(location = 7) in float inp_i_border_thickness;
layout(location = 8) in uint inp_i_shape;
layout(location = 9) in uint inp_i_flags;
layout(location = 0) out vec2 entryPointParam_VS_uv;
layout(location = 1) out vec2 entryPointParam_VS_p;
layout(location = 2) flat out vec2 entryPointParam_VS_size;
layout(location = 3) flat out vec4 entryPointParam_VS_color;
layout(location = 4) flat out vec4 entryPointParam_VS_border_color;
layout(location = 5) flat out vec4 entryPointParam_VS_border_radius;
layout(location = 6) flat out float entryPointParam_VS_border_thickness;
layout(location = 7) flat out uint entryPointParam_VS_shape;
void main()
{
mat4 _74 = mat4(vec4(1.0, 0.0, 0.0, inp_i_position.x), vec4(0.0, 1.0, 0.0, inp_i_position.y), vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
vec2 offset = (-inp_i_offset) * inp_i_size;
_74[0].w = offset.x + inp_i_position.x;
_74[1].w = offset.y + inp_i_position.y;
_74[2].w = 0.0;
_74[3].w = 1.0;
_74[0].x = inp_i_size.x;
_74[1].x = 0.0;
_74[2].x = 0.0;
_74[3].x = 0.0;
_74[0].y = 0.0;
_74[1].y = inp_i_size.y;
_74[2].y = 0.0;
_74[3].y = 0.0;
mat4 _154;
if ((inp_i_flags & 1u) == 1u)
{
_154 = mat4(vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].x, GlobalUniformBuffer.uniforms.screen_projection.data[1].x, GlobalUniformBuffer.uniforms.screen_projection.data[2].x, GlobalUniformBuffer.uniforms.screen_projection.data[3].x), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].y, GlobalUniformBuffer.uniforms.screen_projection.data[1].y, GlobalUniformBuffer.uniforms.screen_projection.data[2].y, GlobalUniformBuffer.uniforms.screen_projection.data[3].y), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].z, GlobalUniformBuffer.uniforms.screen_projection.data[1].z, GlobalUniformBuffer.uniforms.screen_projection.data[2].z, GlobalUniformBuffer.uniforms.screen_projection.data[3].z), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].w, GlobalUniformBuffer.uniforms.screen_projection.data[1].w, GlobalUniformBuffer.uniforms.screen_projection.data[2].w, GlobalUniformBuffer.uniforms.screen_projection.data[3].w));
}
else
{
_154 = mat4(vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].x, GlobalUniformBuffer.uniforms.view_projection.data[1].x, GlobalUniformBuffer.uniforms.view_projection.data[2].x, GlobalUniformBuffer.uniforms.view_projection.data[3].x), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].y, GlobalUniformBuffer.uniforms.view_projection.data[1].y, GlobalUniformBuffer.uniforms.view_projection.data[2].y, GlobalUniformBuffer.uniforms.view_projection.data[3].y), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].z, GlobalUniformBuffer.uniforms.view_projection.data[1].z, GlobalUniformBuffer.uniforms.view_projection.data[2].z, GlobalUniformBuffer.uniforms.view_projection.data[3].z), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].w, GlobalUniformBuffer.uniforms.view_projection.data[1].w, GlobalUniformBuffer.uniforms.view_projection.data[2].w, GlobalUniformBuffer.uniforms.view_projection.data[3].w));
}
vec4 _157 = vec4(inp_position, 0.0, 1.0) * (_74 * _154);
_157.z = inp_i_position.z;
gl_Position = _157;
entryPointParam_VS_uv = inp_position;
entryPointParam_VS_p = (inp_position - vec2(0.5)) * inp_i_size;
entryPointParam_VS_size = inp_i_size;
entryPointParam_VS_color = inp_i_color;
entryPointParam_VS_border_color = inp_i_border_color;
entryPointParam_VS_border_radius = inp_i_border_radius;
entryPointParam_VS_border_thickness = inp_i_border_thickness;
entryPointParam_VS_shape = inp_i_shape;
}
)";

static const char GL_SHAPE_FRAG[4260] = R"(#version 410
layout(location = 0) in vec2 inp_uv;
layout(location = 1) in vec2 inp_p;
layout(location = 2) flat in vec2 inp_size;
layout(location = 3) flat in vec4 inp_color;
layout(location = 4) flat in vec4 inp_border_color;
layout(location = 5) flat in vec4 inp_border_radius;
layout(location = 6) flat in float inp_border_thickness;
layout(location = 7) flat in uint inp_shape;
layout(location = 0) out vec4 entryPointParam_PS;
void main()
{
do
{
vec4 _309;
if (inp_shape == 1u)
{
float radius = 0.5 - (1.0 / inp_size.x);
float _265 = length(inp_uv - vec2(0.5));
float _266 = _265 - radius;
float _267 = fwidth(_266);
float _270 = 1.0 - smoothstep(0.0, _267 * 1.2999999523162841796875, _266);
vec4 _308;
if (inp_border_thickness > 0.0)
{
float _281 = _265 - (radius - (inp_border_thickness / inp_size.x));
float _283 = max(_266, -_281);
float _284 = fwidth(_283);
float _287 = 1.0 - smoothstep(0.0, _284 * 1.2999999523162841796875, _283);
bool _301;
if (_281 > 0.0)
{
_301 = _287 < 1.0;
}
else
{
_301 = false;
}
vec4 _307;
if (_301)
{
_307 = mix(vec4(0.0), inp_border_color, vec4(_287));
}
else
{
_307 = mix(vec4(inp_color.xyz, min(inp_color.w, _270)), inp_border_color, vec4(min(inp_border_color.w, min(_287, _270))));
}
_308 = _307;
}
else
{
_308 = vec4(inp_color.xyz, min(inp_color.w, _270));
}
_309 = _308;
}
else
{
vec4 _195;
if (inp_shape == 2u)
{
float _197 = inp_border_thickness / inp_size.y;
float thickness = 0.5 - _197;
vec2 p = (((vec2(inp_uv.x, 1.0 - inp_uv.y) * 2.0) - vec2(1.0)) * inp_size) / vec2(inp_size.y);
float _206 = _197 + 0.5;
float _251;
do
{
float _211 = atan(p.y, p.x);
float _216 = (_211 - (6.283185482025146484375 * floor(_211 * 0.15915493667125701904296875))) - inp_border_radius.x;
float _222;
if (_216 < 0.0)
{
_222 = _216 + 6.283185482025146484375;
}
else
{
_222 = _216;
}
float _223 = inp_border_radius.y - inp_border_radius.x;
float _229;
if (_223 < 0.0)
{
_229 = _223 + 6.283185482025146484375;
}
else
{
_229 = _223;
}
if (_222 >= _229)
{
_251 = min(length(p - vec2(_206 * cos(inp_border_radius.x), _206 * sin(inp_border_radius.x))), length(p - vec2(_206 * cos(inp_border_radius.y), _206 * sin(inp_border_radius.y))));
break;
}
_251 = abs(length(p) - _206);
break;
} while(false);
entryPointParam_PS = vec4(inp_color.xyz, min(inp_color.w, 1.0 - smoothstep(thickness - length(vec2(dFdx(_251), dFdy(_251))), thickness, _251)));
break;
}
else
{
bool _83;
if (max(inp_border_radius.x, max(inp_border_radius.y, max(inp_border_radius.z, inp_border_radius.w))) > 0.0)
{
_83 = true;
}
else
{
_83 = inp_border_thickness > 0.0;
}
if (_83)
{
vec2 _94;
if (0.0 < inp_p.y)
{
_94 = inp_border_radius.zw;
}
else
{
_94 = inp_border_radius.xy;
}
float _102;
if (0.0 < inp_p.x)
{
_102 = _94.y;
}
else
{
_102 = _94.x;
}
vec2 _104 = inp_size * 0.5;
vec2 _107 = (abs(inp_p) - _104) + vec2(_102);
float _115 = (length(max(_107, vec2(0.0))) + min(max(_107.x, _107.y), 0.0)) - _102;
vec4 _116 = vec4(inp_border_thickness);
vec2 _117 = _116.xy;
vec2 _121 = ((inp_size - _117) - _116.zw) * 0.5;
vec2 _124 = inp_p - ((_117 + _121) - _104);
float _125 = max(inp_border_thickness, inp_border_thickness);
vec4 _136 = min(max(vec4(inp_border_radius.x - _125, inp_border_radius.y - _125, inp_border_radius.z - _125, inp_border_radius.w - _125), vec4(0.0)), vec4(min(_121.x, _121.y)));
vec2 _144;
if (0.0 < _124.y)
{
_144 = _136.zw;
}
else
{
_144 = _136.xy;
}
float _152;
if (0.0 < _124.x)
{
_152 = _144.y;
}
else
{
_152 = _144.x;
}
vec2 _156 = (abs(_124) - _121) + vec2(_152);
float _164 = (length(max(_156, vec2(0.0))) + min(max(_156.x, _156.y), 0.0)) - _152;
float _166 = max(_115, -_164);
float _167 = fwidth(_166);
float _170 = 1.0 - smoothstep(0.0, pow(_167, 0.454545438289642333984375), _166);
float _171 = fwidth(_115);
float _174 = 1.0 - smoothstep(0.0, pow(_171, 0.454545438289642333984375), _115);
bool _188;
if (_164 > 0.0)
{
_188 = _170 < 1.0;
}
else
{
_188 = false;
}
vec4 _194;
if (_188)
{
_194 = vec4(inp_border_color.xyz, _170);
}
else
{
_194 = mix(vec4(inp_color.xyz, min(inp_color.w, _174)), inp_border_color, vec4(min(inp_border_color.w, min(_170, _174))));
}
_195 = _194;
}
else
{
_195 = inp_color;
}
}
_309 = _195;
}
if (_309.w <= 0.00999999977648258209228515625)
{
discard;
}
entryPointParam_PS = _309;
break;
} while(false);
}
)";

static const char D3D11_SPRITE_VERT[4399] = R"(#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#ifndef __DXC_VERSION_MAJOR
#pragma warning(disable : 3557)
#endif
struct GlobalUniforms_0
{
float4x4 screen_projection_0;
float4x4 view_projection_0;
float4x4 nonscale_view_projection_0;
float4x4 nonscale_projection_0;
float4x4 transform_matrix_0;
float4x4 inv_view_proj_0;
float2 camera_position_0;
float2 window_size_0;
};
struct SLANG_ParameterGroup_GlobalUniformBuffer_0
{
GlobalUniforms_0 uniforms_0;
};
cbuffer GlobalUniformBuffer_0 : register(b2)
{
SLANG_ParameterGroup_GlobalUniformBuffer_0 GlobalUniformBuffer_0;
}
struct VSOutput_0
{
float4 position_0 : SV_Position;
nointerpolation float4 color_0 : Color;
nointerpolation float4 outline_color_0 : OutlineColor;
float2 uv_0 : UV;
nointerpolation float outline_thickness_0 : OutlineThickness;
};
struct VSInput_0
{
float2 position_1 : Position;
float3 i_position_0 : I_Position;
float4 i_rotation_0 : I_Rotation;
float2 i_size_0 : I_Size;
float2 i_offset_0 : I_Offset;
float4 i_uv_offset_scale_0 : I_UvOffsetScale;
float4 i_color_0 : I_Color;
float4 i_outline_color_0 : I_OutlineColor;
float i_outline_thickness_0 : I_OutlineThickness;
uint i_flags_0 : I_Flags;
};
VSOutput_0 VS(VSInput_0 inp_0)
{
VSInput_0 _S1 = inp_0;
float _S2 = inp_0.i_rotation_0.x;
float qxx_0 = _S2 * _S2;
float _S3 = inp_0.i_rotation_0.y;
float qyy_0 = _S3 * _S3;
float _S4 = inp_0.i_rotation_0.z;
float qzz_0 = _S4 * _S4;
float qxz_0 = _S2 * _S4;
float qxy_0 = _S2 * _S3;
float qyz_0 = _S3 * _S4;
float _S5 = inp_0.i_rotation_0.w;
float qwx_0 = _S5 * _S2;
float qwy_0 = _S5 * _S3;
float qwz_0 = _S5 * _S4;
float4 _S6 = float4(0.0f, 0.0f, 0.0f, 1.0f);
float4x4 transform_0 = mul(float4x4(float4(1.0f, 0.0f, 0.0f, inp_0.i_position_0.x), float4(0.0f, 1.0f, 0.0f, inp_0.i_position_0.y), float4(0.0f, 0.0f, 1.0f, 0.0f), _S6), float4x4(float4(1.0f - 2.0f * (qyy_0 + qzz_0), 2.0f * (qxy_0 - qwz_0), 2.0f * (qxz_0 + qwy_0), 0.0f), float4(2.0f * (qxy_0 + qwz_0), 1.0f - 2.0f * (qxx_0 + qzz_0), 2.0f * (qyz_0 - qwx_0), 0.0f), float4(2.0f * (qxz_0 - qwy_0), 2.0f * (qyz_0 + qwx_0), 1.0f - 2.0f * (qxx_0 + qyy_0), 0.0f), _S6));
float2 offset_0 = - inp_0.i_offset_0 * inp_0.i_size_0;
transform_0[int(0)][int(3)] = transform_0[int(0)][int(0)] * offset_0[int(0)] + transform_0[int(0)][int(1)] * offset_0[int(1)] + transform_0[int(0)][int(2)] * 0.0f + transform_0[int(0)][int(3)];
transform_0[int(1)][int(3)] = transform_0[int(1)][int(0)] * offset_0[int(0)] + transform_0[int(1)][int(1)] * offset_0[int(1)] + transform_0[int(1)][int(2)] * 0.0f + transform_0[int(1)][int(3)];
transform_0[int(2)][int(3)] = transform_0[int(2)][int(0)] * offset_0[int(0)] + transform_0[int(2)][int(1)] * offset_0[int(1)] + transform_0[int(2)][int(2)] * 0.0f + transform_0[int(2)][int(3)];
transform_0[int(3)][int(3)] = transform_0[int(3)][int(0)] * offset_0[int(0)] + transform_0[int(3)][int(1)] * offset_0[int(1)] + transform_0[int(3)][int(2)] * 0.0f + transform_0[int(3)][int(3)];
transform_0[int(0)][int(0)] = transform_0[int(0)][int(0)] * inp_0.i_size_0[int(0)];
transform_0[int(1)][int(0)] = transform_0[int(1)][int(0)] * inp_0.i_size_0[int(0)];
transform_0[int(2)][int(0)] = transform_0[int(2)][int(0)] * inp_0.i_size_0[int(0)];
transform_0[int(3)][int(0)] = transform_0[int(3)][int(0)] * inp_0.i_size_0[int(0)];
transform_0[int(0)][int(1)] = transform_0[int(0)][int(1)] * inp_0.i_size_0[int(1)];
transform_0[int(1)][int(1)] = transform_0[int(1)][int(1)] * inp_0.i_size_0[int(1)];
transform_0[int(2)][int(1)] = transform_0[int(2)][int(1)] * inp_0.i_size_0[int(1)];
transform_0[int(3)][int(1)] = transform_0[int(3)][int(1)] * inp_0.i_size_0[int(1)];
bool ignore_camera_zoom_0 = (uint(int(inp_0.i_flags_0)) & 2U) == 2U;
float4x4 _S7;
if(((inp_0.i_flags_0) & 1U) == 1U)
{
_S7 = GlobalUniformBuffer_0.uniforms_0.screen_projection_0;
}
else
{
if(ignore_camera_zoom_0)
{
_S7 = GlobalUniformBuffer_0.uniforms_0.nonscale_view_projection_0;
}
else
{
_S7 = GlobalUniformBuffer_0.uniforms_0.view_projection_0;
}
}
VSOutput_0 outp_0;
outp_0.position_0 = mul(mul(_S7, transform_0), float4(_S1.position_1, 0.0f, 1.0f));
outp_0.position_0[int(2)] = _S1.i_position_0.z;
outp_0.uv_0 = _S1.position_1 * _S1.i_uv_offset_scale_0.zw + _S1.i_uv_offset_scale_0.xy;
outp_0.color_0 = _S1.i_color_0;
outp_0.outline_color_0 = _S1.i_outline_color_0;
outp_0.outline_thickness_0 = _S1.i_outline_thickness_0;
return outp_0;
}
)";

static const char D3D11_SPRITE_FRAG[1493] = R"(#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#ifndef __DXC_VERSION_MAJOR
#pragma warning(disable : 3557)
#endif
Texture2D<float4 > Texture_0 : register(t3);
SamplerState Sampler_0 : register(s4);
struct VSOutput_0
{
float4 position_0 : SV_Position;
nointerpolation float4 color_0 : Color;
nointerpolation float4 outline_color_0 : OutlineColor;
float2 uv_0 : UV;
nointerpolation float outline_thickness_0 : OutlineThickness;
};
float4 PS(VSOutput_0 inp_0) : SV_TARGET
{
VSOutput_0 _S1 = inp_0;
float4 color_1;
if((inp_0.outline_thickness_0) > 0.0f)
{
float _S2 = - _S1.outline_thickness_0;
color_1 = lerp(Texture_0.Sample(Sampler_0, _S1.uv_0), _S1.outline_color_0, (float4)min(Texture_0.Sample(Sampler_0, _S1.uv_0 + float2(_S1.outline_thickness_0, 0.0f)).w + Texture_0.Sample(Sampler_0, _S1.uv_0 + float2(_S2, 0.0f)).w + Texture_0.Sample(Sampler_0, _S1.uv_0 + float2(0.0f, _S1.outline_thickness_0)).w + Texture_0.Sample(Sampler_0, _S1.uv_0 + float2(0.0f, _S2)).w + Texture_0.Sample(Sampler_0, _S1.uv_0 + float2(_S1.outline_thickness_0, _S2)).w + Texture_0.Sample(Sampler_0, _S1.uv_0 + float2(_S2, _S1.outline_thickness_0)).w + Texture_0.Sample(Sampler_0, _S1.uv_0 + float2(_S1.outline_thickness_0, _S1.outline_thickness_0)).w + Texture_0.Sample(Sampler_0, _S1.uv_0 + float2(_S2, _S2)).w, 1.0f));
}
else
{
color_1 = Texture_0.Sample(Sampler_0, _S1.uv_0) * _S1.color_0;
}
if((color_1.w) <= 0.02500000037252903f)
{
discard;
}
return color_1;
}
)";

static const unsigned char VULKAN_SPRITE_VERT[7304] = {3, 2, 35, 7, 0, 5, 1, 0, 0, 0, 40, 0, 13, 1, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 21, 0, 0, 0, 0, 0, 1, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 10, 0, 0, 0, 11, 0, 0, 0, 12, 0, 0, 0, 13, 0, 0, 0, 14, 0, 0, 0, 15, 0, 0, 0, 16, 0, 0, 0, 17, 0, 0, 0, 3, 0, 3, 0, 11, 0, 0, 0, 1, 0, 0, 0, 5, 0, 6, 0, 8, 0, 0, 0, 105, 110, 112, 46, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 0, 0, 5, 0, 6, 0, 9, 0, 0, 0, 105, 110, 112, 46, 105, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 6, 0, 10, 0, 0, 0, 105, 110, 112, 46, 105, 95, 114, 111, 116, 97, 116, 105, 111, 110, 0, 0, 5, 0, 5, 0, 11, 0, 0, 0, 105, 110, 112, 46, 105, 95, 115, 105, 122, 101, 0, 0, 5, 0, 6, 0, 12, 0, 0, 0, 105, 110, 112, 46, 105, 95, 111, 102, 102, 115, 101, 116, 0, 0, 0, 0, 5, 0, 8, 0, 13, 0, 0, 0, 105, 110, 112, 46, 105, 95, 117, 118, 95, 111, 102, 102, 115, 101, 116, 95, 115, 99, 97, 108, 101, 0, 0, 0, 5, 0, 5, 0, 14, 0, 0, 0, 105, 110, 112, 46, 105, 95, 99, 111, 108, 111, 114, 0, 5, 0, 7, 0, 15, 0, 0, 0, 105, 110, 112, 46, 105, 95, 111, 117, 116, 108, 105, 110, 101, 95, 99, 111, 108, 111, 114, 0, 5, 0, 8, 0, 16, 0, 0, 0, 105, 110, 112, 46, 105, 95, 111, 117, 116, 108, 105, 110, 101, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 5, 0, 5, 0, 17, 0, 0, 0, 105, 110, 112, 46, 105, 95, 102, 108, 97, 103, 115, 0, 5, 0, 3, 0, 18, 0, 0, 0, 113, 120, 120, 0, 5, 0, 3, 0, 19, 0, 0, 0, 113, 121, 121, 0, 5, 0, 3, 0, 20, 0, 0, 0, 113, 122, 122, 0, 5, 0, 3, 0, 21, 0, 0, 0, 113, 120, 122, 0, 5, 0, 3, 0, 22, 0, 0, 0, 113, 120, 121, 0, 5, 0, 3, 0, 23, 0, 0, 0, 113, 121, 122, 0, 5, 0, 3, 0, 24, 0, 0, 0, 113, 119, 120, 0, 5, 0, 3, 0, 25, 0, 0, 0, 113, 119, 121, 0, 5, 0, 3, 0, 26, 0, 0, 0, 113, 119, 122, 0, 5, 0, 6, 0, 27, 0, 0, 0, 114, 111, 116, 97, 116, 105, 111, 110, 95, 109, 97, 116, 114, 105, 120, 0, 5, 0, 4, 0, 28, 0, 0, 0, 111, 102, 102, 115, 101, 116, 0, 0, 5, 0, 4, 0, 29, 0, 0, 0, 102, 108, 97, 103, 115, 0, 0, 0, 5, 0, 7, 0, 30, 0, 0, 0, 105, 103, 110, 111, 114, 101, 95, 99, 97, 109, 101, 114, 97, 95, 122, 111, 111, 109, 0, 0, 5, 0, 4, 0, 31, 0, 0, 0, 105, 115, 95, 117, 105, 0, 0, 0, 5, 0, 12, 0, 32, 0, 0, 0, 95, 77, 97, 116, 114, 105, 120, 83, 116, 111, 114, 97, 103, 101, 95, 102, 108, 111, 97, 116, 52, 120, 52, 95, 67, 111, 108, 77, 97, 106, 111, 114, 115, 116, 100, 49, 52, 48, 0, 0, 6, 0, 5, 0, 32, 0, 0, 0, 0, 0, 0, 0, 100, 97, 116, 97, 0, 0, 0, 0, 5, 0, 8, 0, 33, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 115, 95, 115, 116, 100, 49, 52, 48, 0, 0, 0, 6, 0, 8, 0, 33, 0, 0, 0, 0, 0, 0, 0, 115, 99, 114, 101, 101, 110, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 7, 0, 33, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 10, 0, 33, 0, 0, 0, 2, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 0, 6, 0, 8, 0, 33, 0, 0, 0, 3, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 8, 0, 33, 0, 0, 0, 4, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 95, 109, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 7, 0, 33, 0, 0, 0, 5, 0, 0, 0, 105, 110, 118, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 0, 0, 0, 6, 0, 7, 0, 33, 0, 0, 0, 6, 0, 0, 0, 99, 97, 109, 101, 114, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 6, 0, 33, 0, 0, 0, 7, 0, 0, 0, 119, 105, 110, 100, 111, 119, 95, 115, 105, 122, 101, 0, 5, 0, 14, 0, 34, 0, 0, 0, 83, 76, 65, 78, 71, 95, 80, 97, 114, 97, 109, 101, 116, 101, 114, 71, 114, 111, 117, 112, 95, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 95, 115, 116, 100, 49, 52, 48, 0, 6, 0, 6, 0, 34, 0, 0, 0, 0, 0, 0, 0, 117, 110, 105, 102, 111, 114, 109, 115, 0, 0, 0, 0, 5, 0, 7, 0, 2, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 5, 0, 9, 0, 4, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 99, 111, 108, 111, 114, 0, 0, 0, 0, 5, 0, 11, 0, 5, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 111, 117, 116, 108, 105, 110, 101, 95, 99, 111, 108, 111, 114, 0, 0, 0, 0, 5, 0, 8, 0, 6, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 117, 118, 0, 0, 0, 5, 0, 12, 0, 7, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 86, 83, 46, 111, 117, 116, 108, 105, 110, 101, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 0, 0, 0, 5, 0, 3, 0, 1, 0, 0, 0, 86, 83, 0, 0, 71, 0, 4, 0, 8, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 9, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 10, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 12, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 13, 0, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 4, 0, 14, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 4, 0, 15, 0, 0, 0, 30, 0, 0, 0, 7, 0, 0, 0, 71, 0, 4, 0, 16, 0, 0, 0, 30, 0, 0, 0, 8, 0, 0, 0, 71, 0, 4, 0, 17, 0, 0, 0, 30, 0, 0, 0, 9, 0, 0, 0, 71, 0, 4, 0, 35, 0, 0, 0, 6, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 32, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 33, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 33, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 5, 0, 33, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 72, 0, 5, 0, 33, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 192, 0, 0, 0, 72, 0, 5, 0, 33, 0, 0, 0, 4, 0, 0, 0, 35, 0, 0, 0, 0, 1, 0, 0, 72, 0, 5, 0, 33, 0, 0, 0, 5, 0, 0, 0, 35, 0, 0, 0, 64, 1, 0, 0, 72, 0, 5, 0, 33, 0, 0, 0, 6, 0, 0, 0, 35, 0, 0, 0, 128, 1, 0, 0, 72, 0, 5, 0, 33, 0, 0, 0, 7, 0, 0, 0, 35, 0, 0, 0, 136, 1, 0, 0, 71, 0, 3, 0, 34, 0, 0, 0, 2, 0, 0, 0, 72, 0, 5, 0, 34, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 2, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 2, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 4, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 4, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 5, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 3, 0, 5, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 7, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 3, 0, 7, 0, 0, 0, 14, 0, 0, 0, 19, 0, 2, 0, 36, 0, 0, 0, 33, 0, 3, 0, 37, 0, 0, 0, 36, 0, 0, 0, 22, 0, 3, 0, 38, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 39, 0, 0, 0, 38, 0, 0, 0, 4, 0, 0, 0, 24, 0, 4, 0, 40, 0, 0, 0, 39, 0, 0, 0, 4, 0, 0, 0, 23, 0, 4, 0, 41, 0, 0, 0, 38, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 42, 0, 0, 0, 1, 0, 0, 0, 41, 0, 0, 0, 23, 0, 4, 0, 43, 0, 0, 0, 38, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 44, 0, 0, 0, 1, 0, 0, 0, 43, 0, 0, 0, 32, 0, 4, 0, 45, 0, 0, 0, 1, 0, 0, 0, 39, 0, 0, 0, 32, 0, 4, 0, 46, 0, 0, 0, 1, 0, 0, 0, 38, 0, 0, 0, 21, 0, 4, 0, 47, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 48, 0, 0, 0, 1, 0, 0, 0, 47, 0, 0, 0, 43, 0, 4, 0, 38, 0, 0, 0, 49, 0, 0, 0, 0, 0, 0, 64, 43, 0, 4, 0, 38, 0, 0, 0, 50, 0, 0, 0, 0, 0, 128, 63, 43, 0, 4, 0, 38, 0, 0, 0, 51, 0, 0, 0, 0, 0, 0, 0, 44, 0, 7, 0, 39, 0, 0, 0, 52, 0, 0, 0, 51, 0, 0, 0, 51, 0, 0, 0, 51, 0, 0, 0, 50, 0, 0, 0, 44, 0, 7, 0, 39, 0, 0, 0, 53, 0, 0, 0, 51, 0, 0, 0, 51, 0, 0, 0, 50, 0, 0, 0, 51, 0, 0, 0, 21, 0, 4, 0, 54, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 54, 0, 0, 0, 55, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 54, 0, 0, 0, 56, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 54, 0, 0, 0, 57, 0, 0, 0, 2, 0, 0, 0, 43, 0, 4, 0, 47, 0, 0, 0, 58, 0, 0, 0, 2, 0, 0, 0, 20, 0, 2, 0, 59, 0, 0, 0, 43, 0, 4, 0, 47, 0, 0, 0, 60, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 54, 0, 0, 0, 61, 0, 0, 0, 4, 0, 0, 0, 28, 0, 4, 0, 35, 0, 0, 0, 39, 0, 0, 0, 61, 0, 0, 0, 30, 0, 3, 0, 32, 0, 0, 0, 35, 0, 0, 0, 30, 0, 10, 0, 33, 0, 0, 0, 32, 0, 0, 0, 32, 0, 0, 0, 32, 0, 0, 0, 32, 0, 0, 0, 32, 0, 0, 0, 32, 0, 0, 0, 41, 0, 0, 0, 41, 0, 0, 0, 30, 0, 3, 0, 34, 0, 0, 0, 33, 0, 0, 0, 32, 0, 4, 0, 62, 0, 0, 0, 2, 0, 0, 0, 34, 0, 0, 0, 32, 0, 4, 0, 63, 0, 0, 0, 2, 0, 0, 0, 32, 0, 0, 0, 32, 0, 4, 0, 64, 0, 0, 0, 3, 0, 0, 0, 39, 0, 0, 0, 32, 0, 4, 0, 65, 0, 0, 0, 3, 0, 0, 0, 41, 0, 0, 0, 32, 0, 4, 0, 66, 0, 0, 0, 3, 0, 0, 0, 38, 0, 0, 0, 59, 0, 4, 0, 42, 0, 0, 0, 8, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 44, 0, 0, 0, 9, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 45, 0, 0, 0, 10, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 42, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 42, 0, 0, 0, 12, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 45, 0, 0, 0, 13, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 45, 0, 0, 0, 14, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 45, 0, 0, 0, 15, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 16, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 48, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 62, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 59, 0, 4, 0, 64, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 64, 0, 0, 0, 4, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 64, 0, 0, 0, 5, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 65, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 66, 0, 0, 0, 7, 0, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 36, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 37, 0, 0, 0, 248, 0, 2, 0, 67, 0, 0, 0, 61, 0, 4, 0, 41, 0, 0, 0, 68, 0, 0, 0, 8, 0, 0, 0, 61, 0, 4, 0, 43, 0, 0, 0, 69, 0, 0, 0, 9, 0, 0, 0, 61, 0, 4, 0, 39, 0, 0, 0, 70, 0, 0, 0, 10, 0, 0, 0, 61, 0, 4, 0, 41, 0, 0, 0, 71, 0, 0, 0, 11, 0, 0, 0, 61, 0, 4, 0, 41, 0, 0, 0, 72, 0, 0, 0, 12, 0, 0, 0, 61, 0, 4, 0, 39, 0, 0, 0, 73, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 39, 0, 0, 0, 74, 0, 0, 0, 14, 0, 0, 0, 61, 0, 4, 0, 39, 0, 0, 0, 75, 0, 0, 0, 15, 0, 0, 0, 61, 0, 4, 0, 38, 0, 0, 0, 76, 0, 0, 0, 16, 0, 0, 0, 61, 0, 4, 0, 47, 0, 0, 0, 77, 0, 0, 0, 17, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 78, 0, 0, 0, 70, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 18, 0, 0, 0, 78, 0, 0, 0, 78, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 79, 0, 0, 0, 70, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 19, 0, 0, 0, 79, 0, 0, 0, 79, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 80, 0, 0, 0, 70, 0, 0, 0, 2, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 20, 0, 0, 0, 80, 0, 0, 0, 80, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 21, 0, 0, 0, 78, 0, 0, 0, 80, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 22, 0, 0, 0, 78, 0, 0, 0, 79, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 23, 0, 0, 0, 79, 0, 0, 0, 80, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 81, 0, 0, 0, 70, 0, 0, 0, 3, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 24, 0, 0, 0, 81, 0, 0, 0, 78, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 25, 0, 0, 0, 81, 0, 0, 0, 79, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 26, 0, 0, 0, 81, 0, 0, 0, 80, 0, 0, 0, 129, 0, 5, 0, 38, 0, 0, 0, 82, 0, 0, 0, 19, 0, 0, 0, 20, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 83, 0, 0, 0, 49, 0, 0, 0, 82, 0, 0, 0, 131, 0, 5, 0, 38, 0, 0, 0, 84, 0, 0, 0, 50, 0, 0, 0, 83, 0, 0, 0, 131, 0, 5, 0, 38, 0, 0, 0, 85, 0, 0, 0, 22, 0, 0, 0, 26, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 86, 0, 0, 0, 49, 0, 0, 0, 85, 0, 0, 0, 129, 0, 5, 0, 38, 0, 0, 0, 87, 0, 0, 0, 21, 0, 0, 0, 25, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 88, 0, 0, 0, 49, 0, 0, 0, 87, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 89, 0, 0, 0, 84, 0, 0, 0, 86, 0, 0, 0, 88, 0, 0, 0, 51, 0, 0, 0, 129, 0, 5, 0, 38, 0, 0, 0, 90, 0, 0, 0, 22, 0, 0, 0, 26, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 91, 0, 0, 0, 49, 0, 0, 0, 90, 0, 0, 0, 129, 0, 5, 0, 38, 0, 0, 0, 92, 0, 0, 0, 18, 0, 0, 0, 20, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 93, 0, 0, 0, 49, 0, 0, 0, 92, 0, 0, 0, 131, 0, 5, 0, 38, 0, 0, 0, 94, 0, 0, 0, 50, 0, 0, 0, 93, 0, 0, 0, 131, 0, 5, 0, 38, 0, 0, 0, 95, 0, 0, 0, 23, 0, 0, 0, 24, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 96, 0, 0, 0, 49, 0, 0, 0, 95, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 97, 0, 0, 0, 91, 0, 0, 0, 94, 0, 0, 0, 96, 0, 0, 0, 51, 0, 0, 0, 131, 0, 5, 0, 38, 0, 0, 0, 98, 0, 0, 0, 21, 0, 0, 0, 25, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 99, 0, 0, 0, 49, 0, 0, 0, 98, 0, 0, 0, 129, 0, 5, 0, 38, 0, 0, 0, 100, 0, 0, 0, 23, 0, 0, 0, 24, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 101, 0, 0, 0, 49, 0, 0, 0, 100, 0, 0, 0, 129, 0, 5, 0, 38, 0, 0, 0, 102, 0, 0, 0, 18, 0, 0, 0, 19, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 103, 0, 0, 0, 49, 0, 0, 0, 102, 0, 0, 0, 131, 0, 5, 0, 38, 0, 0, 0, 104, 0, 0, 0, 50, 0, 0, 0, 103, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 105, 0, 0, 0, 99, 0, 0, 0, 101, 0, 0, 0, 104, 0, 0, 0, 51, 0, 0, 0, 80, 0, 7, 0, 40, 0, 0, 0, 27, 0, 0, 0, 89, 0, 0, 0, 97, 0, 0, 0, 105, 0, 0, 0, 52, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 106, 0, 0, 0, 69, 0, 0, 0, 0, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 107, 0, 0, 0, 50, 0, 0, 0, 51, 0, 0, 0, 51, 0, 0, 0, 106, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 108, 0, 0, 0, 69, 0, 0, 0, 1, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 109, 0, 0, 0, 51, 0, 0, 0, 50, 0, 0, 0, 51, 0, 0, 0, 108, 0, 0, 0, 80, 0, 7, 0, 40, 0, 0, 0, 110, 0, 0, 0, 107, 0, 0, 0, 109, 0, 0, 0, 53, 0, 0, 0, 52, 0, 0, 0, 146, 0, 5, 0, 40, 0, 0, 0, 111, 0, 0, 0, 27, 0, 0, 0, 110, 0, 0, 0, 127, 0, 4, 0, 41, 0, 0, 0, 112, 0, 0, 0, 72, 0, 0, 0, 133, 0, 5, 0, 41, 0, 0, 0, 28, 0, 0, 0, 112, 0, 0, 0, 71, 0, 0, 0, 81, 0, 6, 0, 38, 0, 0, 0, 113, 0, 0, 0, 111, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 114, 0, 0, 0, 28, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 115, 0, 0, 0, 113, 0, 0, 0, 114, 0, 0, 0, 81, 0, 6, 0, 38, 0, 0, 0, 116, 0, 0, 0, 111, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 117, 0, 0, 0, 28, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 118, 0, 0, 0, 116, 0, 0, 0, 117, 0, 0, 0, 129, 0, 5, 0, 38, 0, 0, 0, 119, 0, 0, 0, 115, 0, 0, 0, 118, 0, 0, 0, 81, 0, 6, 0, 38, 0, 0, 0, 120, 0, 0, 0, 111, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 38, 0, 0, 0, 121, 0, 0, 0, 119, 0, 0, 0, 120, 0, 0, 0, 82, 0, 7, 0, 40, 0, 0, 0, 122, 0, 0, 0, 121, 0, 0, 0, 111, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 81, 0, 6, 0, 38, 0, 0, 0, 123, 0, 0, 0, 111, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 124, 0, 0, 0, 123, 0, 0, 0, 114, 0, 0, 0, 81, 0, 6, 0, 38, 0, 0, 0, 125, 0, 0, 0, 111, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 126, 0, 0, 0, 125, 0, 0, 0, 117, 0, 0, 0, 129, 0, 5, 0, 38, 0, 0, 0, 127, 0, 0, 0, 124, 0, 0, 0, 126, 0, 0, 0, 81, 0, 6, 0, 38, 0, 0, 0, 128, 0, 0, 0, 111, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 38, 0, 0, 0, 129, 0, 0, 0, 127, 0, 0, 0, 128, 0, 0, 0, 82, 0, 7, 0, 40, 0, 0, 0, 130, 0, 0, 0, 129, 0, 0, 0, 122, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0, 81, 0, 6, 0, 38, 0, 0, 0, 131, 0, 0, 0, 111, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 132, 0, 0, 0, 131, 0, 0, 0, 114, 0, 0, 0, 81, 0, 6, 0, 38, 0, 0, 0, 133, 0, 0, 0, 111, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 134, 0, 0, 0, 133, 0, 0, 0, 117, 0, 0, 0, 129, 0, 5, 0, 38, 0, 0, 0, 135, 0, 0, 0, 132, 0, 0, 0, 134, 0, 0, 0, 81, 0, 6, 0, 38, 0, 0, 0, 136, 0, 0, 0, 111, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 38, 0, 0, 0, 137, 0, 0, 0, 135, 0, 0, 0, 136, 0, 0, 0, 82, 0, 7, 0, 40, 0, 0, 0, 138, 0, 0, 0, 137, 0, 0, 0, 130, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 81, 0, 6, 0, 38, 0, 0, 0, 139, 0, 0, 0, 111, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 140, 0, 0, 0, 139, 0, 0, 0, 114, 0, 0, 0, 81, 0, 6, 0, 38, 0, 0, 0, 141, 0, 0, 0, 111, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 142, 0, 0, 0, 141, 0, 0, 0, 117, 0, 0, 0, 129, 0, 5, 0, 38, 0, 0, 0, 143, 0, 0, 0, 140, 0, 0, 0, 142, 0, 0, 0, 81, 0, 6, 0, 38, 0, 0, 0, 144, 0, 0, 0, 111, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 38, 0, 0, 0, 145, 0, 0, 0, 143, 0, 0, 0, 144, 0, 0, 0, 82, 0, 7, 0, 40, 0, 0, 0, 146, 0, 0, 0, 145, 0, 0, 0, 138, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 147, 0, 0, 0, 71, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 148, 0, 0, 0, 113, 0, 0, 0, 147, 0, 0, 0, 82, 0, 7, 0, 40, 0, 0, 0, 149, 0, 0, 0, 148, 0, 0, 0, 146, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 150, 0, 0, 0, 123, 0, 0, 0, 147, 0, 0, 0, 82, 0, 7, 0, 40, 0, 0, 0, 151, 0, 0, 0, 150, 0, 0, 0, 149, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 152, 0, 0, 0, 131, 0, 0, 0, 147, 0, 0, 0, 82, 0, 7, 0, 40, 0, 0, 0, 153, 0, 0, 0, 152, 0, 0, 0, 151, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 154, 0, 0, 0, 139, 0, 0, 0, 147, 0, 0, 0, 82, 0, 7, 0, 40, 0, 0, 0, 155, 0, 0, 0, 154, 0, 0, 0, 153, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 156, 0, 0, 0, 71, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 157, 0, 0, 0, 116, 0, 0, 0, 156, 0, 0, 0, 82, 0, 7, 0, 40, 0, 0, 0, 158, 0, 0, 0, 157, 0, 0, 0, 155, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 159, 0, 0, 0, 125, 0, 0, 0, 156, 0, 0, 0, 82, 0, 7, 0, 40, 0, 0, 0, 160, 0, 0, 0, 159, 0, 0, 0, 158, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 161, 0, 0, 0, 133, 0, 0, 0, 156, 0, 0, 0, 82, 0, 7, 0, 40, 0, 0, 0, 162, 0, 0, 0, 161, 0, 0, 0, 160, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 133, 0, 5, 0, 38, 0, 0, 0, 163, 0, 0, 0, 141, 0, 0, 0, 156, 0, 0, 0, 82, 0, 7, 0, 40, 0, 0, 0, 164, 0, 0, 0, 163, 0, 0, 0, 162, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 124, 0, 4, 0, 54, 0, 0, 0, 29, 0, 0, 0, 77, 0, 0, 0, 124, 0, 4, 0, 47, 0, 0, 0, 165, 0, 0, 0, 29, 0, 0, 0, 199, 0, 5, 0, 47, 0, 0, 0, 166, 0, 0, 0, 165, 0, 0, 0, 58, 0, 0, 0, 170, 0, 5, 0, 59, 0, 0, 0, 30, 0, 0, 0, 166, 0, 0, 0, 58, 0, 0, 0, 199, 0, 5, 0, 47, 0, 0, 0, 167, 0, 0, 0, 77, 0, 0, 0, 60, 0, 0, 0, 170, 0, 5, 0, 59, 0, 0, 0, 31, 0, 0, 0, 167, 0, 0, 0, 60, 0, 0, 0, 247, 0, 3, 0, 168, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 31, 0, 0, 0, 169, 0, 0, 0, 170, 0, 0, 0, 248, 0, 2, 0, 170, 0, 0, 0, 247, 0, 3, 0, 171, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 30, 0, 0, 0, 172, 0, 0, 0, 173, 0, 0, 0, 248, 0, 2, 0, 173, 0, 0, 0, 66, 0, 6, 0, 63, 0, 0, 0, 174, 0, 0, 0, 2, 0, 0, 0, 55, 0, 0, 0, 56, 0, 0, 0, 61, 0, 4, 0, 32, 0, 0, 0, 175, 0, 0, 0, 174, 0, 0, 0, 81, 0, 5, 0, 35, 0, 0, 0, 176, 0, 0, 0, 175, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 39, 0, 0, 0, 177, 0, 0, 0, 176, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 178, 0, 0, 0, 177, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 179, 0, 0, 0, 177, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 180, 0, 0, 0, 177, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 181, 0, 0, 0, 177, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 39, 0, 0, 0, 182, 0, 0, 0, 176, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 183, 0, 0, 0, 182, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 184, 0, 0, 0, 182, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 185, 0, 0, 0, 182, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 186, 0, 0, 0, 182, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 39, 0, 0, 0, 187, 0, 0, 0, 176, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 188, 0, 0, 0, 187, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 189, 0, 0, 0, 187, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 190, 0, 0, 0, 187, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 191, 0, 0, 0, 187, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 39, 0, 0, 0, 192, 0, 0, 0, 176, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 193, 0, 0, 0, 192, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 194, 0, 0, 0, 192, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 195, 0, 0, 0, 192, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 196, 0, 0, 0, 192, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 197, 0, 0, 0, 178, 0, 0, 0, 183, 0, 0, 0, 188, 0, 0, 0, 193, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 198, 0, 0, 0, 179, 0, 0, 0, 184, 0, 0, 0, 189, 0, 0, 0, 194, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 199, 0, 0, 0, 180, 0, 0, 0, 185, 0, 0, 0, 190, 0, 0, 0, 195, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 200, 0, 0, 0, 181, 0, 0, 0, 186, 0, 0, 0, 191, 0, 0, 0, 196, 0, 0, 0, 80, 0, 7, 0, 40, 0, 0, 0, 201, 0, 0, 0, 197, 0, 0, 0, 198, 0, 0, 0, 199, 0, 0, 0, 200, 0, 0, 0, 249, 0, 2, 0, 171, 0, 0, 0, 248, 0, 2, 0, 172, 0, 0, 0, 66, 0, 6, 0, 63, 0, 0, 0, 202, 0, 0, 0, 2, 0, 0, 0, 55, 0, 0, 0, 57, 0, 0, 0, 61, 0, 4, 0, 32, 0, 0, 0, 203, 0, 0, 0, 202, 0, 0, 0, 81, 0, 5, 0, 35, 0, 0, 0, 204, 0, 0, 0, 203, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 39, 0, 0, 0, 205, 0, 0, 0, 204, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 206, 0, 0, 0, 205, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 207, 0, 0, 0, 205, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 208, 0, 0, 0, 205, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 209, 0, 0, 0, 205, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 39, 0, 0, 0, 210, 0, 0, 0, 204, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 211, 0, 0, 0, 210, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 212, 0, 0, 0, 210, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 213, 0, 0, 0, 210, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 214, 0, 0, 0, 210, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 39, 0, 0, 0, 215, 0, 0, 0, 204, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 216, 0, 0, 0, 215, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 217, 0, 0, 0, 215, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 218, 0, 0, 0, 215, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 219, 0, 0, 0, 215, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 39, 0, 0, 0, 220, 0, 0, 0, 204, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 221, 0, 0, 0, 220, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 222, 0, 0, 0, 220, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 223, 0, 0, 0, 220, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 224, 0, 0, 0, 220, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 225, 0, 0, 0, 206, 0, 0, 0, 211, 0, 0, 0, 216, 0, 0, 0, 221, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 226, 0, 0, 0, 207, 0, 0, 0, 212, 0, 0, 0, 217, 0, 0, 0, 222, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 227, 0, 0, 0, 208, 0, 0, 0, 213, 0, 0, 0, 218, 0, 0, 0, 223, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 228, 0, 0, 0, 209, 0, 0, 0, 214, 0, 0, 0, 219, 0, 0, 0, 224, 0, 0, 0, 80, 0, 7, 0, 40, 0, 0, 0, 229, 0, 0, 0, 225, 0, 0, 0, 226, 0, 0, 0, 227, 0, 0, 0, 228, 0, 0, 0, 249, 0, 2, 0, 171, 0, 0, 0, 248, 0, 2, 0, 171, 0, 0, 0, 245, 0, 7, 0, 40, 0, 0, 0, 230, 0, 0, 0, 201, 0, 0, 0, 173, 0, 0, 0, 229, 0, 0, 0, 172, 0, 0, 0, 249, 0, 2, 0, 168, 0, 0, 0, 248, 0, 2, 0, 169, 0, 0, 0, 66, 0, 6, 0, 63, 0, 0, 0, 231, 0, 0, 0, 2, 0, 0, 0, 55, 0, 0, 0, 55, 0, 0, 0, 61, 0, 4, 0, 32, 0, 0, 0, 232, 0, 0, 0, 231, 0, 0, 0, 81, 0, 5, 0, 35, 0, 0, 0, 233, 0, 0, 0, 232, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 39, 0, 0, 0, 234, 0, 0, 0, 233, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 235, 0, 0, 0, 234, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 236, 0, 0, 0, 234, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 237, 0, 0, 0, 234, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 238, 0, 0, 0, 234, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 39, 0, 0, 0, 239, 0, 0, 0, 233, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 240, 0, 0, 0, 239, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 241, 0, 0, 0, 239, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 242, 0, 0, 0, 239, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 243, 0, 0, 0, 239, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 39, 0, 0, 0, 244, 0, 0, 0, 233, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 245, 0, 0, 0, 244, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 246, 0, 0, 0, 244, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 247, 0, 0, 0, 244, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 248, 0, 0, 0, 244, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 39, 0, 0, 0, 249, 0, 0, 0, 233, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 250, 0, 0, 0, 249, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 251, 0, 0, 0, 249, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 252, 0, 0, 0, 249, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 253, 0, 0, 0, 249, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 254, 0, 0, 0, 235, 0, 0, 0, 240, 0, 0, 0, 245, 0, 0, 0, 250, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 255, 0, 0, 0, 236, 0, 0, 0, 241, 0, 0, 0, 246, 0, 0, 0, 251, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 0, 1, 0, 0, 237, 0, 0, 0, 242, 0, 0, 0, 247, 0, 0, 0, 252, 0, 0, 0, 80, 0, 7, 0, 39, 0, 0, 0, 1, 1, 0, 0, 238, 0, 0, 0, 243, 0, 0, 0, 248, 0, 0, 0, 253, 0, 0, 0, 80, 0, 7, 0, 40, 0, 0, 0, 2, 1, 0, 0, 254, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 249, 0, 2, 0, 168, 0, 0, 0, 248, 0, 2, 0, 168, 0, 0, 0, 245, 0, 7, 0, 40, 0, 0, 0, 3, 1, 0, 0, 230, 0, 0, 0, 171, 0, 0, 0, 2, 1, 0, 0, 169, 0, 0, 0, 146, 0, 5, 0, 40, 0, 0, 0, 4, 1, 0, 0, 164, 0, 0, 0, 3, 1, 0, 0, 80, 0, 6, 0, 39, 0, 0, 0, 5, 1, 0, 0, 68, 0, 0, 0, 51, 0, 0, 0, 50, 0, 0, 0, 144, 0, 5, 0, 39, 0, 0, 0, 6, 1, 0, 0, 5, 1, 0, 0, 4, 1, 0, 0, 81, 0, 5, 0, 38, 0, 0, 0, 7, 1, 0, 0, 69, 0, 0, 0, 2, 0, 0, 0, 82, 0, 6, 0, 39, 0, 0, 0, 8, 1, 0, 0, 7, 1, 0, 0, 6, 1, 0, 0, 2, 0, 0, 0, 79, 0, 7, 0, 41, 0, 0, 0, 9, 1, 0, 0, 73, 0, 0, 0, 73, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 133, 0, 5, 0, 41, 0, 0, 0, 10, 1, 0, 0, 68, 0, 0, 0, 9, 1, 0, 0, 79, 0, 7, 0, 41, 0, 0, 0, 11, 1, 0, 0, 73, 0, 0, 0, 73, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 129, 0, 5, 0, 41, 0, 0, 0, 12, 1, 0, 0, 10, 1, 0, 0, 11, 1, 0, 0, 62, 0, 3, 0, 3, 0, 0, 0, 8, 1, 0, 0, 62, 0, 3, 0, 4, 0, 0, 0, 74, 0, 0, 0, 62, 0, 3, 0, 5, 0, 0, 0, 75, 0, 0, 0, 62, 0, 3, 0, 6, 0, 0, 0, 12, 1, 0, 0, 62, 0, 3, 0, 7, 0, 0, 0, 76, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_SPRITE_FRAG[3216] = {3, 2, 35, 7, 0, 5, 1, 0, 0, 0, 40, 0, 118, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 12, 0, 4, 0, 0, 0, 2, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 16, 0, 3, 0, 2, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 11, 0, 0, 0, 1, 0, 0, 0, 5, 0, 5, 0, 6, 0, 0, 0, 105, 110, 112, 46, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 7, 0, 7, 0, 0, 0, 105, 110, 112, 46, 111, 117, 116, 108, 105, 110, 101, 95, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 4, 0, 8, 0, 0, 0, 105, 110, 112, 46, 117, 118, 0, 0, 5, 0, 8, 0, 9, 0, 0, 0, 105, 110, 112, 46, 111, 117, 116, 108, 105, 110, 101, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 0, 0, 5, 0, 4, 0, 3, 0, 0, 0, 84, 101, 120, 116, 117, 114, 101, 0, 5, 0, 4, 0, 4, 0, 0, 0, 83, 97, 109, 112, 108, 101, 114, 0, 5, 0, 6, 0, 10, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 73, 109, 97, 103, 101, 0, 0, 0, 0, 5, 0, 4, 0, 11, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 0, 5, 0, 6, 0, 12, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 73, 109, 97, 103, 101, 0, 0, 0, 0, 5, 0, 4, 0, 13, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 0, 5, 0, 4, 0, 14, 0, 0, 0, 111, 117, 116, 108, 105, 110, 101, 0, 5, 0, 6, 0, 15, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 73, 109, 97, 103, 101, 0, 0, 0, 0, 5, 0, 4, 0, 16, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 0, 5, 0, 4, 0, 17, 0, 0, 0, 111, 117, 116, 108, 105, 110, 101, 0, 5, 0, 6, 0, 18, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 73, 109, 97, 103, 101, 0, 0, 0, 0, 5, 0, 4, 0, 19, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 0, 5, 0, 4, 0, 20, 0, 0, 0, 111, 117, 116, 108, 105, 110, 101, 0, 5, 0, 6, 0, 21, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 73, 109, 97, 103, 101, 0, 0, 0, 0, 5, 0, 4, 0, 22, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 0, 5, 0, 4, 0, 23, 0, 0, 0, 111, 117, 116, 108, 105, 110, 101, 0, 5, 0, 6, 0, 24, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 73, 109, 97, 103, 101, 0, 0, 0, 0, 5, 0, 4, 0, 25, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 0, 5, 0, 4, 0, 26, 0, 0, 0, 111, 117, 116, 108, 105, 110, 101, 0, 5, 0, 6, 0, 27, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 73, 109, 97, 103, 101, 0, 0, 0, 0, 5, 0, 4, 0, 28, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 0, 5, 0, 4, 0, 29, 0, 0, 0, 111, 117, 116, 108, 105, 110, 101, 0, 5, 0, 6, 0, 30, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 73, 109, 97, 103, 101, 0, 0, 0, 0, 5, 0, 4, 0, 31, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 0, 5, 0, 4, 0, 32, 0, 0, 0, 111, 117, 116, 108, 105, 110, 101, 0, 5, 0, 6, 0, 33, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 73, 109, 97, 103, 101, 0, 0, 0, 0, 5, 0, 4, 0, 34, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 0, 5, 0, 4, 0, 35, 0, 0, 0, 111, 117, 116, 108, 105, 110, 101, 0, 5, 0, 6, 0, 36, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 73, 109, 97, 103, 101, 0, 0, 0, 0, 5, 0, 4, 0, 37, 0, 0, 0, 115, 97, 109, 112, 108, 101, 100, 0, 5, 0, 7, 0, 5, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 80, 83, 0, 0, 5, 0, 3, 0, 2, 0, 0, 0, 80, 83, 0, 0, 71, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 6, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 7, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 3, 0, 7, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 8, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 9, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 3, 0, 9, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 33, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 4, 0, 0, 0, 33, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 4, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 5, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 38, 0, 0, 0, 33, 0, 3, 0, 39, 0, 0, 0, 38, 0, 0, 0, 22, 0, 3, 0, 40, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 41, 0, 0, 0, 40, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 42, 0, 0, 0, 1, 0, 0, 0, 41, 0, 0, 0, 23, 0, 4, 0, 43, 0, 0, 0, 40, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 44, 0, 0, 0, 1, 0, 0, 0, 43, 0, 0, 0, 32, 0, 4, 0, 45, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 20, 0, 2, 0, 46, 0, 0, 0, 43, 0, 4, 0, 40, 0, 0, 0, 47, 0, 0, 0, 0, 0, 0, 0, 25, 0, 9, 0, 48, 0, 0, 0, 40, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 49, 0, 0, 0, 0, 0, 0, 0, 48, 0, 0, 0, 26, 0, 2, 0, 50, 0, 0, 0, 32, 0, 4, 0, 51, 0, 0, 0, 0, 0, 0, 0, 50, 0, 0, 0, 27, 0, 3, 0, 52, 0, 0, 0, 48, 0, 0, 0, 43, 0, 4, 0, 40, 0, 0, 0, 53, 0, 0, 0, 0, 0, 128, 63, 43, 0, 4, 0, 40, 0, 0, 0, 54, 0, 0, 0, 205, 204, 204, 60, 32, 0, 4, 0, 55, 0, 0, 0, 3, 0, 0, 0, 41, 0, 0, 0, 59, 0, 4, 0, 42, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 42, 0, 0, 0, 7, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 44, 0, 0, 0, 8, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 45, 0, 0, 0, 9, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 49, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 59, 0, 4, 0, 51, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 59, 0, 4, 0, 55, 0, 0, 0, 5, 0, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 38, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 39, 0, 0, 0, 248, 0, 2, 0, 56, 0, 0, 0, 61, 0, 4, 0, 41, 0, 0, 0, 57, 0, 0, 0, 6, 0, 0, 0, 61, 0, 4, 0, 41, 0, 0, 0, 58, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 43, 0, 0, 0, 59, 0, 0, 0, 8, 0, 0, 0, 61, 0, 4, 0, 40, 0, 0, 0, 60, 0, 0, 0, 9, 0, 0, 0, 186, 0, 5, 0, 46, 0, 0, 0, 61, 0, 0, 0, 60, 0, 0, 0, 47, 0, 0, 0, 247, 0, 3, 0, 62, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 61, 0, 0, 0, 63, 0, 0, 0, 64, 0, 0, 0, 248, 0, 2, 0, 64, 0, 0, 0, 61, 0, 4, 0, 48, 0, 0, 0, 65, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 50, 0, 0, 0, 66, 0, 0, 0, 4, 0, 0, 0, 86, 0, 5, 0, 52, 0, 0, 0, 10, 0, 0, 0, 65, 0, 0, 0, 66, 0, 0, 0, 87, 0, 6, 0, 41, 0, 0, 0, 11, 0, 0, 0, 10, 0, 0, 0, 59, 0, 0, 0, 0, 0, 0, 0, 133, 0, 5, 0, 41, 0, 0, 0, 67, 0, 0, 0, 11, 0, 0, 0, 57, 0, 0, 0, 249, 0, 2, 0, 62, 0, 0, 0, 248, 0, 2, 0, 63, 0, 0, 0, 80, 0, 5, 0, 43, 0, 0, 0, 68, 0, 0, 0, 60, 0, 0, 0, 47, 0, 0, 0, 129, 0, 5, 0, 43, 0, 0, 0, 69, 0, 0, 0, 59, 0, 0, 0, 68, 0, 0, 0, 61, 0, 4, 0, 48, 0, 0, 0, 70, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 50, 0, 0, 0, 71, 0, 0, 0, 4, 0, 0, 0, 86, 0, 5, 0, 52, 0, 0, 0, 12, 0, 0, 0, 70, 0, 0, 0, 71, 0, 0, 0, 87, 0, 6, 0, 41, 0, 0, 0, 13, 0, 0, 0, 12, 0, 0, 0, 69, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 14, 0, 0, 0, 13, 0, 0, 0, 3, 0, 0, 0, 127, 0, 4, 0, 40, 0, 0, 0, 72, 0, 0, 0, 60, 0, 0, 0, 80, 0, 5, 0, 43, 0, 0, 0, 73, 0, 0, 0, 72, 0, 0, 0, 47, 0, 0, 0, 129, 0, 5, 0, 43, 0, 0, 0, 74, 0, 0, 0, 59, 0, 0, 0, 73, 0, 0, 0, 61, 0, 4, 0, 48, 0, 0, 0, 75, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 50, 0, 0, 0, 76, 0, 0, 0, 4, 0, 0, 0, 86, 0, 5, 0, 52, 0, 0, 0, 15, 0, 0, 0, 75, 0, 0, 0, 76, 0, 0, 0, 87, 0, 6, 0, 41, 0, 0, 0, 16, 0, 0, 0, 15, 0, 0, 0, 74, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 77, 0, 0, 0, 16, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 17, 0, 0, 0, 14, 0, 0, 0, 77, 0, 0, 0, 80, 0, 5, 0, 43, 0, 0, 0, 78, 0, 0, 0, 47, 0, 0, 0, 60, 0, 0, 0, 129, 0, 5, 0, 43, 0, 0, 0, 79, 0, 0, 0, 59, 0, 0, 0, 78, 0, 0, 0, 61, 0, 4, 0, 48, 0, 0, 0, 80, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 50, 0, 0, 0, 81, 0, 0, 0, 4, 0, 0, 0, 86, 0, 5, 0, 52, 0, 0, 0, 18, 0, 0, 0, 80, 0, 0, 0, 81, 0, 0, 0, 87, 0, 6, 0, 41, 0, 0, 0, 19, 0, 0, 0, 18, 0, 0, 0, 79, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 82, 0, 0, 0, 19, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 20, 0, 0, 0, 17, 0, 0, 0, 82, 0, 0, 0, 80, 0, 5, 0, 43, 0, 0, 0, 83, 0, 0, 0, 47, 0, 0, 0, 72, 0, 0, 0, 129, 0, 5, 0, 43, 0, 0, 0, 84, 0, 0, 0, 59, 0, 0, 0, 83, 0, 0, 0, 61, 0, 4, 0, 48, 0, 0, 0, 85, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 50, 0, 0, 0, 86, 0, 0, 0, 4, 0, 0, 0, 86, 0, 5, 0, 52, 0, 0, 0, 21, 0, 0, 0, 85, 0, 0, 0, 86, 0, 0, 0, 87, 0, 6, 0, 41, 0, 0, 0, 22, 0, 0, 0, 21, 0, 0, 0, 84, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 87, 0, 0, 0, 22, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 23, 0, 0, 0, 20, 0, 0, 0, 87, 0, 0, 0, 80, 0, 5, 0, 43, 0, 0, 0, 88, 0, 0, 0, 60, 0, 0, 0, 72, 0, 0, 0, 129, 0, 5, 0, 43, 0, 0, 0, 89, 0, 0, 0, 59, 0, 0, 0, 88, 0, 0, 0, 61, 0, 4, 0, 48, 0, 0, 0, 90, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 50, 0, 0, 0, 91, 0, 0, 0, 4, 0, 0, 0, 86, 0, 5, 0, 52, 0, 0, 0, 24, 0, 0, 0, 90, 0, 0, 0, 91, 0, 0, 0, 87, 0, 6, 0, 41, 0, 0, 0, 25, 0, 0, 0, 24, 0, 0, 0, 89, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 92, 0, 0, 0, 25, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 26, 0, 0, 0, 23, 0, 0, 0, 92, 0, 0, 0, 80, 0, 5, 0, 43, 0, 0, 0, 93, 0, 0, 0, 72, 0, 0, 0, 60, 0, 0, 0, 129, 0, 5, 0, 43, 0, 0, 0, 94, 0, 0, 0, 59, 0, 0, 0, 93, 0, 0, 0, 61, 0, 4, 0, 48, 0, 0, 0, 95, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 50, 0, 0, 0, 96, 0, 0, 0, 4, 0, 0, 0, 86, 0, 5, 0, 52, 0, 0, 0, 27, 0, 0, 0, 95, 0, 0, 0, 96, 0, 0, 0, 87, 0, 6, 0, 41, 0, 0, 0, 28, 0, 0, 0, 27, 0, 0, 0, 94, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 97, 0, 0, 0, 28, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 29, 0, 0, 0, 26, 0, 0, 0, 97, 0, 0, 0, 80, 0, 5, 0, 43, 0, 0, 0, 98, 0, 0, 0, 60, 0, 0, 0, 60, 0, 0, 0, 129, 0, 5, 0, 43, 0, 0, 0, 99, 0, 0, 0, 59, 0, 0, 0, 98, 0, 0, 0, 61, 0, 4, 0, 48, 0, 0, 0, 100, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 50, 0, 0, 0, 101, 0, 0, 0, 4, 0, 0, 0, 86, 0, 5, 0, 52, 0, 0, 0, 30, 0, 0, 0, 100, 0, 0, 0, 101, 0, 0, 0, 87, 0, 6, 0, 41, 0, 0, 0, 31, 0, 0, 0, 30, 0, 0, 0, 99, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 102, 0, 0, 0, 31, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 32, 0, 0, 0, 29, 0, 0, 0, 102, 0, 0, 0, 80, 0, 5, 0, 43, 0, 0, 0, 103, 0, 0, 0, 72, 0, 0, 0, 72, 0, 0, 0, 129, 0, 5, 0, 43, 0, 0, 0, 104, 0, 0, 0, 59, 0, 0, 0, 103, 0, 0, 0, 61, 0, 4, 0, 48, 0, 0, 0, 105, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 50, 0, 0, 0, 106, 0, 0, 0, 4, 0, 0, 0, 86, 0, 5, 0, 52, 0, 0, 0, 33, 0, 0, 0, 105, 0, 0, 0, 106, 0, 0, 0, 87, 0, 6, 0, 41, 0, 0, 0, 34, 0, 0, 0, 33, 0, 0, 0, 104, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 107, 0, 0, 0, 34, 0, 0, 0, 3, 0, 0, 0, 129, 0, 5, 0, 40, 0, 0, 0, 35, 0, 0, 0, 32, 0, 0, 0, 107, 0, 0, 0, 12, 0, 7, 0, 40, 0, 0, 0, 108, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 35, 0, 0, 0, 53, 0, 0, 0, 61, 0, 4, 0, 48, 0, 0, 0, 109, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 50, 0, 0, 0, 110, 0, 0, 0, 4, 0, 0, 0, 86, 0, 5, 0, 52, 0, 0, 0, 36, 0, 0, 0, 109, 0, 0, 0, 110, 0, 0, 0, 87, 0, 6, 0, 41, 0, 0, 0, 37, 0, 0, 0, 36, 0, 0, 0, 59, 0, 0, 0, 0, 0, 0, 0, 80, 0, 7, 0, 41, 0, 0, 0, 111, 0, 0, 0, 108, 0, 0, 0, 108, 0, 0, 0, 108, 0, 0, 0, 108, 0, 0, 0, 12, 0, 8, 0, 41, 0, 0, 0, 112, 0, 0, 0, 1, 0, 0, 0, 46, 0, 0, 0, 37, 0, 0, 0, 58, 0, 0, 0, 111, 0, 0, 0, 249, 0, 2, 0, 62, 0, 0, 0, 248, 0, 2, 0, 62, 0, 0, 0, 245, 0, 7, 0, 41, 0, 0, 0, 113, 0, 0, 0, 67, 0, 0, 0, 64, 0, 0, 0, 112, 0, 0, 0, 63, 0, 0, 0, 81, 0, 5, 0, 40, 0, 0, 0, 114, 0, 0, 0, 113, 0, 0, 0, 3, 0, 0, 0, 188, 0, 5, 0, 46, 0, 0, 0, 115, 0, 0, 0, 114, 0, 0, 0, 54, 0, 0, 0, 247, 0, 3, 0, 116, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 115, 0, 0, 0, 117, 0, 0, 0, 116, 0, 0, 0, 248, 0, 2, 0, 117, 0, 0, 0, 252, 0, 1, 0, 248, 0, 2, 0, 116, 0, 0, 0, 62, 0, 3, 0, 5, 0, 0, 0, 113, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const char METAL_SPRITE_VERT[9985] = R"(#include <metal_stdlib>
#include <metal_math>
#include <metal_texture>
using namespace metal;
struct VS_Result_0
{
float4 position_0 [[position]];
float4 color_0 [[user(COLOR)]];
float4 outline_color_0 [[user(OUTLINECOLOR)]];
float2 uv_0 [[user(UV)]];
float outline_thickness_0 [[user(OUTLINETHICKNESS)]];
};
struct vertexInput_0
{
float2 position_1 [[attribute(0)]];
float3 i_position_0 [[attribute(1)]];
float4 i_rotation_0 [[attribute(2)]];
float2 i_size_0 [[attribute(3)]];
float2 i_offset_0 [[attribute(4)]];
float4 i_uv_offset_scale_0 [[attribute(5)]];
float4 i_color_0 [[attribute(6)]];
float4 i_outline_color_0 [[attribute(7)]];
float i_outline_thickness_0 [[attribute(8)]];
uint i_flags_0 [[attribute(9)]];
};
struct _MatrixStorage_float4x4_ColMajornatural_0
{
array<float4, int(4)> data_0;
};
struct GlobalUniforms_natural_0
{
_MatrixStorage_float4x4_ColMajornatural_0 screen_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 view_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 nonscale_view_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 nonscale_projection_0;
_MatrixStorage_float4x4_ColMajornatural_0 transform_matrix_0;
_MatrixStorage_float4x4_ColMajornatural_0 inv_view_proj_0;
float2 camera_position_0;
float2 window_size_0;
};
struct SLANG_ParameterGroup_GlobalUniformBuffer_natural_0
{
GlobalUniforms_natural_0 uniforms_0;
};
struct KernelContext_0
{
SLANG_ParameterGroup_GlobalUniformBuffer_natural_0 constant* GlobalUniformBuffer_0;
};
struct VSOutput_0
{
float4 position_2;
[[flat]] float4 color_1;
[[flat]] float4 outline_color_1;
float2 uv_1;
[[flat]] float outline_thickness_1;
};
[[vertex]] VS_Result_0 VS(vertexInput_0 _S1 [[stage_in]], SLANG_ParameterGroup_GlobalUniformBuffer_natural_0 constant* GlobalUniformBuffer_1 [[buffer(2)]])
{
KernelContext_0 kernelContext_0;
(&kernelContext_0)->GlobalUniformBuffer_0 = GlobalUniformBuffer_1;
float _S2 = _S1.i_rotation_0.x;
float qxx_0 = _S2 * _S2;
float _S3 = _S1.i_rotation_0.y;
float qyy_0 = _S3 * _S3;
float _S4 = _S1.i_rotation_0.z;
float qzz_0 = _S4 * _S4;
float qxz_0 = _S2 * _S4;
float qxy_0 = _S2 * _S3;
float qyz_0 = _S3 * _S4;
float _S5 = _S1.i_rotation_0.w;
float qwx_0 = _S5 * _S2;
float qwy_0 = _S5 * _S3;
float qwz_0 = _S5 * _S4;
float4 _S6 = float4(0.0, 0.0, 0.0, 1.0);
thread matrix<float,int(4),int(4)>  transform_0 = (((matrix<float,int(4),int(4)> (float4(1.0 - 2.0 * (qyy_0 + qzz_0), 2.0 * (qxy_0 - qwz_0), 2.0 * (qxz_0 + qwy_0), 0.0), float4(2.0 * (qxy_0 + qwz_0), 1.0 - 2.0 * (qxx_0 + qzz_0), 2.0 * (qyz_0 - qwx_0), 0.0), float4(2.0 * (qxz_0 - qwy_0), 2.0 * (qyz_0 + qwx_0), 1.0 - 2.0 * (qxx_0 + qyy_0), 0.0), _S6)) * (matrix<float,int(4),int(4)> (float4(1.0, 0.0, 0.0, _S1.i_position_0.x), float4(0.0, 1.0, 0.0, _S1.i_position_0.y), float4(0.0, 0.0, 1.0, 0.0), _S6))));
float2 offset_0 = - _S1.i_offset_0 * _S1.i_size_0;
transform_0[int(0)].w = transform_0[int(0)].x * offset_0[int(0)] + transform_0[int(0)].y * offset_0[int(1)] + transform_0[int(0)].z * 0.0 + transform_0[int(0)].w;
transform_0[int(1)].w = transform_0[int(1)].x * offset_0[int(0)] + transform_0[int(1)].y * offset_0[int(1)] + transform_0[int(1)].z * 0.0 + transform_0[int(1)].w;
transform_0[int(2)].w = transform_0[int(2)].x * offset_0[int(0)] + transform_0[int(2)].y * offset_0[int(1)] + transform_0[int(2)].z * 0.0 + transform_0[int(2)].w;
transform_0[int(3)].w = transform_0[int(3)].x * offset_0[int(0)] + transform_0[int(3)].y * offset_0[int(1)] + transform_0[int(3)].z * 0.0 + transform_0[int(3)].w;
transform_0[int(0)].x = transform_0[int(0)].x * _S1.i_size_0[int(0)];
transform_0[int(1)].x = transform_0[int(1)].x * _S1.i_size_0[int(0)];
transform_0[int(2)].x = transform_0[int(2)].x * _S1.i_size_0[int(0)];
transform_0[int(3)].x = transform_0[int(3)].x * _S1.i_size_0[int(0)];
transform_0[int(0)].y = transform_0[int(0)].y * _S1.i_size_0[int(1)];
transform_0[int(1)].y = transform_0[int(1)].y * _S1.i_size_0[int(1)];
transform_0[int(2)].y = transform_0[int(2)].y * _S1.i_size_0[int(1)];
transform_0[int(3)].y = transform_0[int(3)].y * _S1.i_size_0[int(1)];
bool ignore_camera_zoom_0 = (uint(int(_S1.i_flags_0)) & 2U) == 2U;
matrix<float,int(4),int(4)>  _S7;
if(((_S1.i_flags_0) & 1U) == 1U)
{
_S7 = matrix<float,int(4),int(4)> ((&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(0)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(1)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(2)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->screen_projection_0.data_0[int(3)][int(3)]);
}
else
{
if(ignore_camera_zoom_0)
{
_S7 = matrix<float,int(4),int(4)> ((&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(0)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(1)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(2)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(3)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(0)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(1)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(2)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(3)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(0)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(1)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(2)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(3)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(0)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(1)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(2)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->nonscale_view_projection_0.data_0[int(3)][int(3)]);
}
else
{
_S7 = matrix<float,int(4),int(4)> ((&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(0)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(1)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(2)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(0)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(1)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(2)][int(3)], (&(&kernelContext_0)->GlobalUniformBuffer_0->uniforms_0)->view_projection_0.data_0[int(3)][int(3)]);
}
}
thread VSOutput_0 outp_0;
(&outp_0)->position_2 = (((float4(_S1.position_1, 0.0, 1.0)) * ((((transform_0) * (_S7))))));
(&outp_0)->position_2.z = _S1.i_position_0.z;
(&outp_0)->uv_1 = _S1.position_1 * _S1.i_uv_offset_scale_0.zw + _S1.i_uv_offset_scale_0.xy;
(&outp_0)->color_1 = _S1.i_color_0;
(&outp_0)->outline_color_1 = _S1.i_outline_color_0;
(&outp_0)->outline_thickness_1 = _S1.i_outline_thickness_0;
VSOutput_0 _S8 = outp_0;
thread VS_Result_0 _S9;
(&_S9)->position_0 = _S8.position_2;
(&_S9)->color_0 = _S8.color_1;
(&_S9)->outline_color_0 = _S8.outline_color_1;
(&_S9)->uv_0 = _S8.uv_1;
(&_S9)->outline_thickness_0 = _S8.outline_thickness_1;
return _S9;
}
)";

static const char METAL_SPRITE_FRAG[2218] = R"(#include <metal_stdlib>
#include <metal_math>
#include <metal_texture>
using namespace metal;
struct pixelOutput_0
{
float4 output_0 [[color(0)]];
};
struct pixelInput_0
{
[[flat]] float4 color_0 [[user(COLOR)]];
[[flat]] float4 outline_color_0 [[user(OUTLINECOLOR)]];
float2 uv_0 [[user(UV)]];
[[flat]] float outline_thickness_0 [[user(OUTLINETHICKNESS)]];
};
struct KernelContext_0
{
texture2d<float, access::sample> Texture_0;
sampler Sampler_0;
};
[[fragment]] pixelOutput_0 PS(pixelInput_0 _S1 [[stage_in]], float4 position_0 [[position]], texture2d<float, access::sample> Texture_1 [[texture(3)]], sampler Sampler_1 [[sampler(4)]])
{
KernelContext_0 kernelContext_0;
(&kernelContext_0)->Texture_0 = Texture_1;
(&kernelContext_0)->Sampler_0 = Sampler_1;
float4 color_1;
if((_S1.outline_thickness_0) > 0.0)
{
float _S2 = - _S1.outline_thickness_0;
color_1 = mix((((&kernelContext_0)->Texture_0).sample(((&kernelContext_0)->Sampler_0), (_S1.uv_0))), _S1.outline_color_0, float4(min((((&kernelContext_0)->Texture_0).sample(((&kernelContext_0)->Sampler_0), (_S1.uv_0 + float2(_S1.outline_thickness_0, 0.0)))).w + (((&kernelContext_0)->Texture_0).sample(((&kernelContext_0)->Sampler_0), (_S1.uv_0 + float2(_S2, 0.0)))).w + (((&kernelContext_0)->Texture_0).sample(((&kernelContext_0)->Sampler_0), (_S1.uv_0 + float2(0.0, _S1.outline_thickness_0)))).w + (((&kernelContext_0)->Texture_0).sample(((&kernelContext_0)->Sampler_0), (_S1.uv_0 + float2(0.0, _S2)))).w + (((&kernelContext_0)->Texture_0).sample(((&kernelContext_0)->Sampler_0), (_S1.uv_0 + float2(_S1.outline_thickness_0, _S2)))).w + (((&kernelContext_0)->Texture_0).sample(((&kernelContext_0)->Sampler_0), (_S1.uv_0 + float2(_S2, _S1.outline_thickness_0)))).w + (((&kernelContext_0)->Texture_0).sample(((&kernelContext_0)->Sampler_0), (_S1.uv_0 + float2(_S1.outline_thickness_0, _S1.outline_thickness_0)))).w + (((&kernelContext_0)->Texture_0).sample(((&kernelContext_0)->Sampler_0), (_S1.uv_0 + float2(_S2, _S2)))).w, 1.0)) );
}
else
{
color_1 = (((&kernelContext_0)->Texture_0).sample(((&kernelContext_0)->Sampler_0), (_S1.uv_0))) * _S1.color_0;
}
if((color_1.w) <= 0.02500000037252903)
{
discard_fragment();
}
pixelOutput_0 _S3 = { color_1 };
return _S3;
}
)";

static const char GL_SPRITE_VERT[6449] = R"(#version 410
struct _MatrixStorage_float4x4_ColMajorstd140
{
vec4 data[4];
};
struct GlobalUniforms_std140
{
_MatrixStorage_float4x4_ColMajorstd140 screen_projection;
_MatrixStorage_float4x4_ColMajorstd140 view_projection;
_MatrixStorage_float4x4_ColMajorstd140 nonscale_view_projection;
_MatrixStorage_float4x4_ColMajorstd140 nonscale_projection;
_MatrixStorage_float4x4_ColMajorstd140 transform_matrix;
_MatrixStorage_float4x4_ColMajorstd140 inv_view_proj;
vec2 camera_position;
vec2 window_size;
};
layout(std140) uniform GlobalUniformBuffer_std140
{
GlobalUniforms_std140 uniforms;
} GlobalUniformBuffer;
layout(location = 0) in vec2 inp_position;
layout(location = 1) in vec3 inp_i_position;
layout(location = 2) in vec4 inp_i_rotation;
layout(location = 3) in vec2 inp_i_size;
layout(location = 4) in vec2 inp_i_offset;
layout(location = 5) in vec4 inp_i_uv_offset_scale;
layout(location = 6) in vec4 inp_i_color;
layout(location = 7) in vec4 inp_i_outline_color;
layout(location = 8) in float inp_i_outline_thickness;
layout(location = 9) in uint inp_i_flags;
layout(location = 0) flat out vec4 entryPointParam_VS_color;
layout(location = 1) flat out vec4 entryPointParam_VS_outline_color;
layout(location = 2) out vec2 entryPointParam_VS_uv;
layout(location = 3) flat out float entryPointParam_VS_outline_thickness;
void main()
{
float qxx = inp_i_rotation.x * inp_i_rotation.x;
float qyy = inp_i_rotation.y * inp_i_rotation.y;
float qzz = inp_i_rotation.z * inp_i_rotation.z;
float qxz = inp_i_rotation.x * inp_i_rotation.z;
float qxy = inp_i_rotation.x * inp_i_rotation.y;
float qyz = inp_i_rotation.y * inp_i_rotation.z;
float qwx = inp_i_rotation.w * inp_i_rotation.x;
float qwy = inp_i_rotation.w * inp_i_rotation.y;
float qwz = inp_i_rotation.w * inp_i_rotation.z;
mat4 _111 = mat4(vec4(1.0 - (2.0 * (qyy + qzz)), 2.0 * (qxy - qwz), 2.0 * (qxz + qwy), 0.0), vec4(2.0 * (qxy + qwz), 1.0 - (2.0 * (qxx + qzz)), 2.0 * (qyz - qwx), 0.0), vec4(2.0 * (qxz - qwy), 2.0 * (qyz + qwx), 1.0 - (2.0 * (qxx + qyy)), 0.0), vec4(0.0, 0.0, 0.0, 1.0)) * mat4(vec4(1.0, 0.0, 0.0, inp_i_position.x), vec4(0.0, 1.0, 0.0, inp_i_position.y), vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
vec2 offset = (-inp_i_offset) * inp_i_size;
float _113 = _111[0].x;
float _114 = offset.x;
float _116 = _111[0].y;
float _117 = offset.y;
mat4 _122 = _111;
_122[0].w = ((_113 * _114) + (_116 * _117)) + _111[0].w;
float _123 = _111[1].x;
float _125 = _111[1].y;
_122[1].w = ((_123 * _114) + (_125 * _117)) + _111[1].w;
float _131 = _111[2].x;
float _133 = _111[2].y;
_122[2].w = ((_131 * _114) + (_133 * _117)) + _111[2].w;
float _139 = _111[3].x;
float _141 = _111[3].y;
_122[3].w = ((_139 * _114) + (_141 * _117)) + _111[3].w;
_122[0].x = _113 * inp_i_size.x;
_122[1].x = _123 * inp_i_size.x;
_122[2].x = _131 * inp_i_size.x;
_122[3].x = _139 * inp_i_size.x;
_122[0].y = _116 * inp_i_size.y;
_122[1].y = _125 * inp_i_size.y;
_122[2].y = _133 * inp_i_size.y;
_122[3].y = _141 * inp_i_size.y;
mat4 _259;
if ((inp_i_flags & 1u) == 1u)
{
_259 = mat4(vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].x, GlobalUniformBuffer.uniforms.screen_projection.data[1].x, GlobalUniformBuffer.uniforms.screen_projection.data[2].x, GlobalUniformBuffer.uniforms.screen_projection.data[3].x), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].y, GlobalUniformBuffer.uniforms.screen_projection.data[1].y, GlobalUniformBuffer.uniforms.screen_projection.data[2].y, GlobalUniformBuffer.uniforms.screen_projection.data[3].y), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].z, GlobalUniformBuffer.uniforms.screen_projection.data[1].z, GlobalUniformBuffer.uniforms.screen_projection.data[2].z, GlobalUniformBuffer.uniforms.screen_projection.data[3].z), vec4(GlobalUniformBuffer.uniforms.screen_projection.data[0].w, GlobalUniformBuffer.uniforms.screen_projection.data[1].w, GlobalUniformBuffer.uniforms.screen_projection.data[2].w, GlobalUniformBuffer.uniforms.screen_projection.data[3].w));
}
else
{
mat4 _230;
if ((uint(int(inp_i_flags)) & 2u) == 2u)
{
_230 = mat4(vec4(GlobalUniformBuffer.uniforms.nonscale_view_projection.data[0].x, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[1].x, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[2].x, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[3].x), vec4(GlobalUniformBuffer.uniforms.nonscale_view_projection.data[0].y, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[1].y, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[2].y, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[3].y), vec4(GlobalUniformBuffer.uniforms.nonscale_view_projection.data[0].z, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[1].z, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[2].z, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[3].z), vec4(GlobalUniformBuffer.uniforms.nonscale_view_projection.data[0].w, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[1].w, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[2].w, GlobalUniformBuffer.uniforms.nonscale_view_projection.data[3].w));
}
else
{
_230 = mat4(vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].x, GlobalUniformBuffer.uniforms.view_projection.data[1].x, GlobalUniformBuffer.uniforms.view_projection.data[2].x, GlobalUniformBuffer.uniforms.view_projection.data[3].x), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].y, GlobalUniformBuffer.uniforms.view_projection.data[1].y, GlobalUniformBuffer.uniforms.view_projection.data[2].y, GlobalUniformBuffer.uniforms.view_projection.data[3].y), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].z, GlobalUniformBuffer.uniforms.view_projection.data[1].z, GlobalUniformBuffer.uniforms.view_projection.data[2].z, GlobalUniformBuffer.uniforms.view_projection.data[3].z), vec4(GlobalUniformBuffer.uniforms.view_projection.data[0].w, GlobalUniformBuffer.uniforms.view_projection.data[1].w, GlobalUniformBuffer.uniforms.view_projection.data[2].w, GlobalUniformBuffer.uniforms.view_projection.data[3].w));
}
_259 = _230;
}
vec4 _262 = vec4(inp_position, 0.0, 1.0) * (_122 * _259);
_262.z = inp_i_position.z;
gl_Position = _262;
entryPointParam_VS_color = inp_i_color;
entryPointParam_VS_outline_color = inp_i_outline_color;
entryPointParam_VS_uv = (inp_position * inp_i_uv_offset_scale.zw) + inp_i_uv_offset_scale.xy;
entryPointParam_VS_outline_thickness = inp_i_outline_thickness;
}
)";

static const char GL_SPRITE_FRAG[1057] = R"(#version 410
uniform sampler2D Texture;
layout(location = 0) flat in vec4 inp_color;
layout(location = 1) flat in vec4 inp_outline_color;
layout(location = 2) in vec2 inp_uv;
layout(location = 3) flat in float inp_outline_thickness;
layout(location = 0) out vec4 entryPointParam_PS;
void main()
{
vec4 _113;
if (inp_outline_thickness > 0.0)
{
float _72 = -inp_outline_thickness;
_113 = mix(texture(Texture, inp_uv), inp_outline_color, vec4(min(((((((texture(Texture, inp_uv + vec2(inp_outline_thickness, 0.0)).w + texture(Texture, inp_uv + vec2(_72, 0.0)).w) + texture(Texture, inp_uv + vec2(0.0, inp_outline_thickness)).w) + texture(Texture, inp_uv + vec2(0.0, _72)).w) + texture(Texture, inp_uv + vec2(inp_outline_thickness, _72)).w) + texture(Texture, inp_uv + vec2(_72, inp_outline_thickness)).w) + texture(Texture, inp_uv + vec2(inp_outline_thickness)).w) + texture(Texture, inp_uv + vec2(_72)).w, 1.0)));
}
else
{
_113 = texture(Texture, inp_uv) * inp_color;
}
if (_113.w <= 0.02500000037252902984619140625)
{
discard;
}
entryPointParam_PS = _113;
}
)";

struct ShaderSourceCode {
    ShaderSourceCode(const void* v, size_t vs, const void* f, size_t fs) :
        vs_source(v),
        vs_size(vs),
        fs_source(f),
        fs_size(fs) {}

    const void* vs_source;
    size_t vs_size;

    const void* fs_source;
    size_t fs_size;
};

static inline ShaderSourceCode GetFontShaderSourceCode(const sge::RenderBackend backend) {
    switch (backend) {
        case sge::RenderBackend::Vulkan: return ShaderSourceCode(VULKAN_FONT_VERT, sizeof(VULKAN_FONT_VERT), VULKAN_FONT_FRAG, sizeof(VULKAN_FONT_FRAG));
        case sge::RenderBackend::D3D11:
        case sge::RenderBackend::D3D12: return ShaderSourceCode(D3D11_FONT_VERT, sizeof(D3D11_FONT_VERT), D3D11_FONT_FRAG, sizeof(D3D11_FONT_FRAG));
        case sge::RenderBackend::Metal: return ShaderSourceCode(METAL_FONT_VERT, sizeof(METAL_FONT_VERT), METAL_FONT_FRAG, sizeof(METAL_FONT_FRAG));
        case sge::RenderBackend::OpenGL: return ShaderSourceCode(GL_FONT_VERT, sizeof(GL_FONT_VERT), GL_FONT_FRAG, sizeof(GL_FONT_FRAG));
        default: SGE_UNREACHABLE();
    }
}
static inline ShaderSourceCode GetLineShaderSourceCode(const sge::RenderBackend backend) {
    switch (backend) {
        case sge::RenderBackend::Vulkan: return ShaderSourceCode(VULKAN_LINE_VERT, sizeof(VULKAN_LINE_VERT), VULKAN_LINE_FRAG, sizeof(VULKAN_LINE_FRAG));
        case sge::RenderBackend::D3D11:
        case sge::RenderBackend::D3D12: return ShaderSourceCode(D3D11_LINE_VERT, sizeof(D3D11_LINE_VERT), D3D11_LINE_FRAG, sizeof(D3D11_LINE_FRAG));
        case sge::RenderBackend::Metal: return ShaderSourceCode(METAL_LINE_VERT, sizeof(METAL_LINE_VERT), METAL_LINE_FRAG, sizeof(METAL_LINE_FRAG));
        case sge::RenderBackend::OpenGL: return ShaderSourceCode(GL_LINE_VERT, sizeof(GL_LINE_VERT), GL_LINE_FRAG, sizeof(GL_LINE_FRAG));
        default: SGE_UNREACHABLE();
    }
}
static inline ShaderSourceCode GetNinepatchShaderSourceCode(const sge::RenderBackend backend) {
    switch (backend) {
        case sge::RenderBackend::Vulkan: return ShaderSourceCode(VULKAN_NINEPATCH_VERT, sizeof(VULKAN_NINEPATCH_VERT), VULKAN_NINEPATCH_FRAG, sizeof(VULKAN_NINEPATCH_FRAG));
        case sge::RenderBackend::D3D11:
        case sge::RenderBackend::D3D12: return ShaderSourceCode(D3D11_NINEPATCH_VERT, sizeof(D3D11_NINEPATCH_VERT), D3D11_NINEPATCH_FRAG, sizeof(D3D11_NINEPATCH_FRAG));
        case sge::RenderBackend::Metal: return ShaderSourceCode(METAL_NINEPATCH_VERT, sizeof(METAL_NINEPATCH_VERT), METAL_NINEPATCH_FRAG, sizeof(METAL_NINEPATCH_FRAG));
        case sge::RenderBackend::OpenGL: return ShaderSourceCode(GL_NINEPATCH_VERT, sizeof(GL_NINEPATCH_VERT), GL_NINEPATCH_FRAG, sizeof(GL_NINEPATCH_FRAG));
        default: SGE_UNREACHABLE();
    }
}
static inline ShaderSourceCode GetShapeShaderSourceCode(const sge::RenderBackend backend) {
    switch (backend) {
        case sge::RenderBackend::Vulkan: return ShaderSourceCode(VULKAN_SHAPE_VERT, sizeof(VULKAN_SHAPE_VERT), VULKAN_SHAPE_FRAG, sizeof(VULKAN_SHAPE_FRAG));
        case sge::RenderBackend::D3D11:
        case sge::RenderBackend::D3D12: return ShaderSourceCode(D3D11_SHAPE_VERT, sizeof(D3D11_SHAPE_VERT), D3D11_SHAPE_FRAG, sizeof(D3D11_SHAPE_FRAG));
        case sge::RenderBackend::Metal: return ShaderSourceCode(METAL_SHAPE_VERT, sizeof(METAL_SHAPE_VERT), METAL_SHAPE_FRAG, sizeof(METAL_SHAPE_FRAG));
        case sge::RenderBackend::OpenGL: return ShaderSourceCode(GL_SHAPE_VERT, sizeof(GL_SHAPE_VERT), GL_SHAPE_FRAG, sizeof(GL_SHAPE_FRAG));
        default: SGE_UNREACHABLE();
    }
}
static inline ShaderSourceCode GetSpriteShaderSourceCode(const sge::RenderBackend backend) {
    switch (backend) {
        case sge::RenderBackend::Vulkan: return ShaderSourceCode(VULKAN_SPRITE_VERT, sizeof(VULKAN_SPRITE_VERT), VULKAN_SPRITE_FRAG, sizeof(VULKAN_SPRITE_FRAG));
        case sge::RenderBackend::D3D11:
        case sge::RenderBackend::D3D12: return ShaderSourceCode(D3D11_SPRITE_VERT, sizeof(D3D11_SPRITE_VERT), D3D11_SPRITE_FRAG, sizeof(D3D11_SPRITE_FRAG));
        case sge::RenderBackend::Metal: return ShaderSourceCode(METAL_SPRITE_VERT, sizeof(METAL_SPRITE_VERT), METAL_SPRITE_FRAG, sizeof(METAL_SPRITE_FRAG));
        case sge::RenderBackend::OpenGL: return ShaderSourceCode(GL_SPRITE_VERT, sizeof(GL_SPRITE_VERT), GL_SPRITE_FRAG, sizeof(GL_SPRITE_FRAG));
        default: SGE_UNREACHABLE();
    }
}
#endif