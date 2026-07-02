#include "common.slang"

[vk::binding(2)]
cbuffer GlobalUniformBuffer : register( b2 )
{
    GlobalUniforms uniforms;
};

struct VSInput
{
    float2 position : Position;

    float3 i_color : I_Color;
    float2 i_position : I_Position;
    float2 i_size : I_Size;
    float2 i_em_size : I_Em_Size;
    uint i_partition_offset : I_PartitionOffset;
    uint i_partition_count : I_PartitionCount;
    uint i_flags : I_Flags;
};

struct VSOutput
{
    float4 position : SV_Position;

    float2 uv : UV;
    nointerpolation float2 em_size : EmSize;
    nointerpolation float3 color : Color;
    uint partition_offset : PartitionOffset;
    uint partition_count : PartitionCount;
};

static const uint FLAG_UI = 1 << 0;

[shader("vertex")]
VSOutput VS(VSInput inp)
{
    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = is_ui ? uniforms.screen_projection : uniforms.view_projection;

    float2 position = mad(inp.position, inp.i_size, inp.i_position);
    position.x -= frac(position.x);
    position.y -= frac(position.y);

    VSOutput outp;
    outp.color = inp.i_color;
    outp.uv = inp.position * inp.i_em_size;
    outp.position = mul(mvp, float4(position, 0.0, 1.0));
    outp.position.z = 1.0f;
    outp.em_size = inp.i_em_size;
    outp.partition_offset = inp.i_partition_offset;
    outp.partition_count = inp.i_partition_count;

	return outp;
}

float IntersectMonotonicBezier(float qa, float c0, float c1, float c2, float target)
{
    if (abs(qa) < 1e-3f) {
        return (target - c0) / (c2 - c0);
    }

    float qb = mad(2.h, c1, -2.h * c0);
    float qc = c0 - target;

    float d = mad(qb, qb, -4.h * qa * qc);

    float sqrt_d = select(d < 0.h, 0.h, sqrt(d));

    float inv_2a = 0.5h / qa;

    return mad(-qb, inv_2a, sign(c2 - c0) * sqrt_d * inv_2a);
}

float2 EvaluateBezier(float2 p0, float2 p1, float2 p2, float t)
{
    float2 a = lerp(p0, p1, t);
    float2 b = lerp(p1, p2, t);
    return lerp(a, b, t);
}

float ScanlineSweep(float2 size, float2 offset, float2 p0, float2 p1, float2 p2)
{
    if (max(p0.y, p2.y) <= offset.y || min(p0.y, p2.y) >= offset.y + size.y) {
        return 0.h;
    }

    float2 delta = p2 - p0;

    p0 -= offset;
    p1 -= offset;
    p2 -= offset;

    if (p0.x == p1.x && p0.x == p2.x) {
        if (p0.x >= size.x) {
            return 0.h;
        }

        float t = min(max(p0.y, p2.y), size.y);
        float b = max(min(p0.y, p2.y), 0.h);

        float h = abs(t - b);
        float base = min(size.x, size.x - p0.x);

        return sign(delta.y) * base * h;
    }

    float qa = mad(-2.h, p1.y, p0.y + p2.y);
    float bt = IntersectMonotonicBezier(qa, p0.y, p1.y, p2.y, 0.);
    float tt = IntersectMonotonicBezier(qa, p0.y, p1.y, p2.y, size.y);

    float v_min_t = select(delta.y > 0.h, bt, tt);
    float v_max_t = select(delta.y > 0.h, tt, bt);

    float2 v_min = EvaluateBezier(p0, p1, p2, saturate(v_min_t));
    float2 v_max = EvaluateBezier(p0, p1, p2, saturate(v_max_t));

    if (max(v_min.x, v_max.x) <= 0.h) {
        // Fast path for curves entirely to the left of the window
        // within the scanline. Note that the area sign is
        // incorporated in the result.
        return (v_max.y - v_min.y) * size.x;
    }
    if (min(v_min.x, v_max.x) >= size.x) {
        // The curve is entirely to the right of the window within
        // the scanline, so it can be ignored.
        return 0.h;
    }

    qa = mad(-2.h, p1.x, p0.x + p2.x);

    float h_min_t;
    float h_max_t;

    float4 h_check = select(delta.x > 0.h, float4(p0.x, p2.x, 0.h, 0.h), float4(p2.x, p0.x, size.x, 1.h));

    if (h_check.x >= h_check.z) {
        h_min_t = h_check.w;
    } else if (h_check.y <= h_check.z) {
        h_min_t = 1.h - h_check.w;
    } else {
        h_min_t = IntersectMonotonicBezier(qa, p0.x, p1.x, p2.x, h_check.z);
    }

    h_check.z = size.x - h_check.z;

    if (h_check.x >= h_check.z) {
        h_max_t = h_check.w;
    } else if (h_check.y <= h_check.z) {
        h_max_t = 1.h - h_check.w;
    } else {
        h_max_t = IntersectMonotonicBezier(qa, p0.x, p1.x, p2.x, h_check.z);
    }

    float min_t = saturate(max(v_min_t, h_min_t));
    float max_t = saturate(min(v_max_t, h_max_t));
    
    float2 q0 = v_min_t >= h_min_t ? v_min : EvaluateBezier(p0, p1, p2, min_t);
    float2 q1 = v_max_t <= h_max_t ? v_max : EvaluateBezier(p0, p1, p2, max_t);

    float coverage = 0.h;
    if (min_t > 0.h && delta.x > 0.h) {
        float h = select(delta.y > 0.h, q0.y - max(0.h, p0.y), min(size.y, p0.y) - q0.y);
        coverage = sign(delta.y) * h * size.x;
    }

    if (max_t < 1.h && delta.x < 0.h) {
        float h = select(delta.y > 0.h, min(size.y, p2.y) - q1.y, q1.y - max(0.f, p2.y));
        coverage += sign(delta.y) * h * size.x;
    }

    float h = q1.y - q0.y;
    float b = mad(-0.5h, q0.x + q1.x, size.x);

    coverage += b * h;

    return coverage;
}

struct BezierCurve {
    uint p0;
    uint p1;
    uint p2;
};

struct BezierCurvePartition {
    uint curve_offset;
    uint curve_count;
};

[vk::binding(3)]
StructuredBuffer<BezierCurve> CurveBuffer : register(t3);

[vk::binding(4)]
StructuredBuffer<BezierCurvePartition> PartitionBuffer : register(t4);

static constexpr float PARTITION_HEIGHT = 75.f;

inline float2 UnpackPoint(uint packed)
{
    return float2(packed & 0xFFFFu, (packed >> 16) & 0xFFFFu);
}

[shader("fragment")]
float4 PS(VSOutput inp) : SV_Target
{
    float2 pixel_size = fwidth(inp.uv);

    float y_lo = inp.uv.y - pixel_size.y;
    float y_hi = inp.uv.y + pixel_size.y;

    int p_lo = clamp((int)floor(y_lo / PARTITION_HEIGHT), 0, int(inp.partition_count) - 1);
    int p_hi = clamp((int)floor(y_hi / PARTITION_HEIGHT), 0, int(inp.partition_count) - 1);

    float coverage = 0.h;

    for (int partition_index = p_lo; partition_index <= p_hi; ++partition_index) {
        BezierCurvePartition partition = PartitionBuffer[inp.partition_offset + partition_index];
        
        for (uint i = 0; i < partition.curve_count; ++i) {
            BezierCurve curve = CurveBuffer[partition.curve_offset + i];

            // uint16 points are packed in 32-bit integers, unpacking them
            float2 p0 = UnpackPoint(curve.p0);
            float2 p1 = UnpackPoint(curve.p1);
            float2 p2 = UnpackPoint(curve.p2);

            coverage += ScanlineSweep(pixel_size, inp.uv, p2, p1, p0);
        }
    }

    float pixel_area = pixel_size.x * pixel_size.y;
    coverage /= pixel_area;
    
    float alpha = pow(saturate(coverage), 1.f / 2.2f);

    return float4(inp.color * alpha, alpha);
};

