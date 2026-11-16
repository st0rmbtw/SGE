struct GlobalUniforms
{
    float4x4 screen_projection;
    float4x4 view_projection;
    float4x4 inv_view_projection;
    float2 camera_position;
    uint2 window_size;
};

float4 UnpackRGBA8Unorm(uint packed)
{
    uint4 bytes = uint4(
        packed & 0xFF,
        (packed >> 8) & 0xFF,
        (packed >> 16) & 0xFF,
        (packed >> 24) & 0xFF
    );
    return float4(bytes) / 255.0;
}