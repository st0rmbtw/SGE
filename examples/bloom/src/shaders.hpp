#ifndef SHADERS_HPP_
#define SHADERS_HPP_

#include <cstdlib>
#include <SGE/types/backend.hpp>
#include <SGE/assert.hpp>

static const char D3D11_BASIC_VERT[790] = R"(#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#ifndef __DXC_VERSION_MAJOR
#pragma warning(disable : 3557)
#endif
struct Uniforms_0
{
float4x4 viewMatrix_0;
float4x4 projectionMatrix_0;
float3 color_0;
};
struct SLANG_ParameterGroup_UniformConstantBuffer_0
{
Uniforms_0 uniforms_0;
};
cbuffer UniformConstantBuffer_0 : register(b2)
{
SLANG_ParameterGroup_UniformConstantBuffer_0 UniformConstantBuffer_0;
}
struct VSOutput_0
{
float4 position_0 : SV_Position;
};
struct VSInput_0
{
float3 pos_0 : Position;
};
VSOutput_0 VS(VSInput_0 inp_0)
{
VSOutput_0 outp_0;
outp_0.position_0 = mul(mul(UniformConstantBuffer_0.uniforms_0.projectionMatrix_0, UniformConstantBuffer_0.uniforms_0.viewMatrix_0), float4(inp_0.pos_0, 1.0f));
return outp_0;
}
)";

static const char D3D11_BASIC_FRAG[622] = R"(#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#ifndef __DXC_VERSION_MAJOR
#pragma warning(disable : 3557)
#endif
struct Uniforms_0
{
float4x4 viewMatrix_0;
float4x4 projectionMatrix_0;
float3 color_0;
};
struct SLANG_ParameterGroup_UniformConstantBuffer_0
{
Uniforms_0 uniforms_0;
};
cbuffer UniformConstantBuffer_0 : register(b2)
{
SLANG_ParameterGroup_UniformConstantBuffer_0 UniformConstantBuffer_0;
}
struct VSOutput_0
{
float4 position_0 : SV_Position;
};
float4 PS(VSOutput_0 inp_0) : SV_TARGET
{
return float4(UniformConstantBuffer_0.uniforms_0.color_0, 1.0f);
}
)";

static const unsigned char VULKAN_BASIC_VERT[1048] = {3, 2, 35, 7, 0, 5, 1, 0, 0, 0, 40, 0, 30, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 8, 0, 0, 0, 0, 0, 1, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 3, 0, 3, 0, 11, 0, 0, 0, 1, 0, 0, 0, 5, 0, 6, 0, 5, 0, 0, 0, 85, 110, 105, 102, 111, 114, 109, 115, 95, 115, 116, 100, 49, 52, 48, 0, 6, 0, 6, 0, 5, 0, 0, 0, 0, 0, 0, 0, 118, 105, 101, 119, 77, 97, 116, 114, 105, 120, 0, 0, 6, 0, 8, 0, 5, 0, 0, 0, 1, 0, 0, 0, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 77, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 5, 0, 5, 0, 0, 0, 2, 0, 0, 0, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 15, 0, 6, 0, 0, 0, 83, 76, 65, 78, 71, 95, 80, 97, 114, 97, 109, 101, 116, 101, 114, 71, 114, 111, 117, 112, 95, 85, 110, 105, 102, 111, 114, 109, 67, 111, 110, 115, 116, 97, 110, 116, 66, 117, 102, 102, 101, 114, 95, 115, 116, 100, 49, 52, 48, 0, 0, 0, 6, 0, 6, 0, 6, 0, 0, 0, 0, 0, 0, 0, 117, 110, 105, 102, 111, 114, 109, 115, 0, 0, 0, 0, 5, 0, 8, 0, 2, 0, 0, 0, 85, 110, 105, 102, 111, 114, 109, 67, 111, 110, 115, 116, 97, 110, 116, 66, 117, 102, 102, 101, 114, 0, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 105, 110, 112, 46, 112, 111, 115, 0, 5, 0, 3, 0, 1, 0, 0, 0, 86, 83, 0, 0, 72, 0, 5, 0, 5, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 4, 0, 5, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 72, 0, 5, 0, 5, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 5, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 4, 0, 5, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 72, 0, 5, 0, 5, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 5, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 71, 0, 3, 0, 6, 0, 0, 0, 2, 0, 0, 0, 72, 0, 5, 0, 6, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 2, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 2, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 4, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 7, 0, 0, 0, 33, 0, 3, 0, 8, 0, 0, 0, 7, 0, 0, 0, 22, 0, 3, 0, 9, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 10, 0, 0, 0, 9, 0, 0, 0, 4, 0, 0, 0, 21, 0, 4, 0, 11, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 11, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 24, 0, 4, 0, 13, 0, 0, 0, 10, 0, 0, 0, 4, 0, 0, 0, 23, 0, 4, 0, 14, 0, 0, 0, 9, 0, 0, 0, 3, 0, 0, 0, 30, 0, 5, 0, 5, 0, 0, 0, 13, 0, 0, 0, 13, 0, 0, 0, 14, 0, 0, 0, 30, 0, 3, 0, 6, 0, 0, 0, 5, 0, 0, 0, 32, 0, 4, 0, 15, 0, 0, 0, 2, 0, 0, 0, 6, 0, 0, 0, 43, 0, 4, 0, 11, 0, 0, 0, 16, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 17, 0, 0, 0, 2, 0, 0, 0, 13, 0, 0, 0, 32, 0, 4, 0, 18, 0, 0, 0, 1, 0, 0, 0, 14, 0, 0, 0, 43, 0, 4, 0, 9, 0, 0, 0, 19, 0, 0, 0, 0, 0, 128, 63, 32, 0, 4, 0, 20, 0, 0, 0, 3, 0, 0, 0, 10, 0, 0, 0, 59, 0, 4, 0, 15, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 59, 0, 4, 0, 18, 0, 0, 0, 4, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 20, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 7, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 248, 0, 2, 0, 21, 0, 0, 0, 65, 0, 6, 0, 17, 0, 0, 0, 22, 0, 0, 0, 2, 0, 0, 0, 12, 0, 0, 0, 16, 0, 0, 0, 61, 0, 4, 0, 13, 0, 0, 0, 23, 0, 0, 0, 22, 0, 0, 0, 65, 0, 6, 0, 17, 0, 0, 0, 24, 0, 0, 0, 2, 0, 0, 0, 12, 0, 0, 0, 12, 0, 0, 0, 61, 0, 4, 0, 13, 0, 0, 0, 25, 0, 0, 0, 24, 0, 0, 0, 146, 0, 5, 0, 13, 0, 0, 0, 26, 0, 0, 0, 25, 0, 0, 0, 23, 0, 0, 0, 61, 0, 4, 0, 14, 0, 0, 0, 27, 0, 0, 0, 4, 0, 0, 0, 80, 0, 5, 0, 10, 0, 0, 0, 28, 0, 0, 0, 27, 0, 0, 0, 19, 0, 0, 0, 144, 0, 5, 0, 10, 0, 0, 0, 29, 0, 0, 0, 28, 0, 0, 0, 26, 0, 0, 0, 62, 0, 3, 0, 3, 0, 0, 0, 29, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_BASIC_FRAG[924] = {3, 2, 35, 7, 0, 5, 1, 0, 0, 0, 40, 0, 23, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 7, 0, 4, 0, 0, 0, 1, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 16, 0, 3, 0, 1, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 11, 0, 0, 0, 1, 0, 0, 0, 5, 0, 6, 0, 4, 0, 0, 0, 85, 110, 105, 102, 111, 114, 109, 115, 95, 115, 116, 100, 49, 52, 48, 0, 6, 0, 6, 0, 4, 0, 0, 0, 0, 0, 0, 0, 118, 105, 101, 119, 77, 97, 116, 114, 105, 120, 0, 0, 6, 0, 8, 0, 4, 0, 0, 0, 1, 0, 0, 0, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 77, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 5, 0, 4, 0, 0, 0, 2, 0, 0, 0, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 15, 0, 5, 0, 0, 0, 83, 76, 65, 78, 71, 95, 80, 97, 114, 97, 109, 101, 116, 101, 114, 71, 114, 111, 117, 112, 95, 85, 110, 105, 102, 111, 114, 109, 67, 111, 110, 115, 116, 97, 110, 116, 66, 117, 102, 102, 101, 114, 95, 115, 116, 100, 49, 52, 48, 0, 0, 0, 6, 0, 6, 0, 5, 0, 0, 0, 0, 0, 0, 0, 117, 110, 105, 102, 111, 114, 109, 115, 0, 0, 0, 0, 5, 0, 8, 0, 2, 0, 0, 0, 85, 110, 105, 102, 111, 114, 109, 67, 111, 110, 115, 116, 97, 110, 116, 66, 117, 102, 102, 101, 114, 0, 0, 0, 5, 0, 7, 0, 3, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 95, 80, 83, 0, 0, 5, 0, 3, 0, 1, 0, 0, 0, 80, 83, 0, 0, 72, 0, 5, 0, 4, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 4, 0, 4, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 72, 0, 5, 0, 4, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 4, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 4, 0, 4, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 72, 0, 5, 0, 4, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 4, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 71, 0, 3, 0, 5, 0, 0, 0, 2, 0, 0, 0, 72, 0, 5, 0, 5, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 2, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 2, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 3, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 6, 0, 0, 0, 33, 0, 3, 0, 7, 0, 0, 0, 6, 0, 0, 0, 22, 0, 3, 0, 8, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 9, 0, 0, 0, 8, 0, 0, 0, 4, 0, 0, 0, 24, 0, 4, 0, 10, 0, 0, 0, 9, 0, 0, 0, 4, 0, 0, 0, 23, 0, 4, 0, 11, 0, 0, 0, 8, 0, 0, 0, 3, 0, 0, 0, 30, 0, 5, 0, 4, 0, 0, 0, 10, 0, 0, 0, 10, 0, 0, 0, 11, 0, 0, 0, 30, 0, 3, 0, 5, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 12, 0, 0, 0, 2, 0, 0, 0, 5, 0, 0, 0, 21, 0, 4, 0, 13, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 13, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 13, 0, 0, 0, 15, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 16, 0, 0, 0, 2, 0, 0, 0, 11, 0, 0, 0, 43, 0, 4, 0, 8, 0, 0, 0, 17, 0, 0, 0, 0, 0, 128, 63, 32, 0, 4, 0, 18, 0, 0, 0, 3, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 12, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 59, 0, 4, 0, 18, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 6, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 248, 0, 2, 0, 19, 0, 0, 0, 65, 0, 6, 0, 16, 0, 0, 0, 20, 0, 0, 0, 2, 0, 0, 0, 14, 0, 0, 0, 15, 0, 0, 0, 61, 0, 4, 0, 11, 0, 0, 0, 21, 0, 0, 0, 20, 0, 0, 0, 80, 0, 5, 0, 9, 0, 0, 0, 22, 0, 0, 0, 21, 0, 0, 0, 17, 0, 0, 0, 62, 0, 3, 0, 3, 0, 0, 0, 22, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const char METAL_BASIC_VERT[3547] = R"(#include <metal_stdlib>
#include <metal_math>
#include <metal_texture>
using namespace metal;
struct VS_Result_0
{
float4 position_0 [[position]];
};
struct vertexInput_0
{
float3 pos_0 [[attribute(0)]];
};
struct _MatrixStorage_float4x4_ColMajornatural_0
{
array<float4, int(4)> data_0;
};
struct Uniforms_natural_0
{
_MatrixStorage_float4x4_ColMajornatural_0 viewMatrix_0;
_MatrixStorage_float4x4_ColMajornatural_0 projectionMatrix_0;
float3 color_0;
};
struct SLANG_ParameterGroup_UniformConstantBuffer_natural_0
{
Uniforms_natural_0 uniforms_0;
};
struct VSOutput_0
{
float4 position_1;
};
vertex VS_Result_0 VS(vertexInput_0 _S1 [[stage_in]], SLANG_ParameterGroup_UniformConstantBuffer_natural_0 constant* UniformConstantBuffer_0 [[buffer(2)]])
{
thread VSOutput_0 outp_0;
(&outp_0)->position_1 = (((float4(_S1.pos_0, 1.0)) * ((((matrix<float,int(4),int(4)> ((&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(0)][int(0)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(1)][int(0)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(2)][int(0)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(3)][int(0)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(0)][int(1)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(1)][int(1)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(2)][int(1)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(3)][int(1)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(0)][int(2)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(1)][int(2)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(2)][int(2)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(3)][int(2)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(0)][int(3)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(1)][int(3)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(2)][int(3)], (&UniformConstantBuffer_0->uniforms_0)->viewMatrix_0.data_0[int(3)][int(3)])) * (matrix<float,int(4),int(4)> ((&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(0)][int(0)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(1)][int(0)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(2)][int(0)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(3)][int(0)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(0)][int(1)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(1)][int(1)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(2)][int(1)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(3)][int(1)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(0)][int(2)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(1)][int(2)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(2)][int(2)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(3)][int(2)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(0)][int(3)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(1)][int(3)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(2)][int(3)], (&UniformConstantBuffer_0->uniforms_0)->projectionMatrix_0.data_0[int(3)][int(3)])))))));
thread VS_Result_0 _S2;
(&_S2)->position_0 = outp_0.position_1;
return _S2;
}
)";

static const char METAL_BASIC_FRAG[762] = R"(#include <metal_stdlib>
#include <metal_math>
#include <metal_texture>
using namespace metal;
struct pixelOutput_0
{
float4 output_0 [[color(0)]];
};
struct _MatrixStorage_float4x4_ColMajornatural_0
{
array<float4, int(4)> data_0;
};
struct Uniforms_natural_0
{
_MatrixStorage_float4x4_ColMajornatural_0 viewMatrix_0;
_MatrixStorage_float4x4_ColMajornatural_0 projectionMatrix_0;
float3 color_0;
};
struct SLANG_ParameterGroup_UniformConstantBuffer_natural_0
{
Uniforms_natural_0 uniforms_0;
};
fragment pixelOutput_0 PS(float4 position_0 [[position]], SLANG_ParameterGroup_UniformConstantBuffer_natural_0 constant* UniformConstantBuffer_0 [[buffer(2)]])
{
pixelOutput_0 _S1 = { float4((&UniformConstantBuffer_0->uniforms_0)->color_0, 1.0) };
return _S1;
}
)";

static const char GL_BASIC_VERT[502] = R"(#version 410
struct Uniforms_std140
{
mat4 viewMatrix;
mat4 projectionMatrix;
vec3 color;
};
layout(std140) uniform UniformConstantBuffer_std140
{
layout(row_major) Uniforms_std140 uniforms;
} UniformConstantBuffer;
layout(location = 0) in vec3 inp_pos;
mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }
void main()
{
gl_Position = vec4(inp_pos, 1.0) * (spvWorkaroundRowMajor(UniformConstantBuffer.uniforms.viewMatrix) * spvWorkaroundRowMajor(UniformConstantBuffer.uniforms.projectionMatrix));
}
)";

static const char GL_BASIC_FRAG[335] = R"(#version 410
struct Uniforms_std140
{
mat4 viewMatrix;
mat4 projectionMatrix;
vec3 color;
};
layout(std140) uniform UniformConstantBuffer_std140
{
layout(row_major) Uniforms_std140 uniforms;
} UniformConstantBuffer;
layout(location = 0) out vec4 fragColor;
void main()
{
fragColor = vec4(UniformConstantBuffer.uniforms.color, 1.0);
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

static inline ShaderSourceCode GetBasicShaderSourceCode(const sge::RenderBackend backend) {
    switch (backend) {
        case sge::RenderBackend::Vulkan: return ShaderSourceCode(VULKAN_BASIC_VERT, sizeof(VULKAN_BASIC_VERT), VULKAN_BASIC_FRAG, sizeof(VULKAN_BASIC_FRAG));
        case sge::RenderBackend::D3D11:
        case sge::RenderBackend::D3D12: return ShaderSourceCode(D3D11_BASIC_VERT, sizeof(D3D11_BASIC_VERT), D3D11_BASIC_FRAG, sizeof(D3D11_BASIC_FRAG));
        case sge::RenderBackend::Metal: return ShaderSourceCode(METAL_BASIC_VERT, sizeof(METAL_BASIC_VERT), METAL_BASIC_FRAG, sizeof(METAL_BASIC_FRAG));
        case sge::RenderBackend::OpenGL: return ShaderSourceCode(GL_BASIC_VERT, sizeof(GL_BASIC_VERT), GL_BASIC_FRAG, sizeof(GL_BASIC_FRAG));
        default: SGE_UNREACHABLE();
    }
}
#endif // SHADERS_HPP_