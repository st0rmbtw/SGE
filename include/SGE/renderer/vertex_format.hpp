#ifndef SGE_RENDERER_VERTEX_FORMAT_HPP_
#define SGE_RENDERER_VERTEX_FORMAT_HPP_

#include <cstdint>

namespace sge {

enum class VertexFormat : uint8_t {
    Uint8 = 0,
    Uint8x2,
    Uint8x3,
    Uint8x4,
    Sint8,
    Sint8x2,
    Sint8x3,
    Sint8x4,
    Unorm8,
    Unorm8x2,
    Unorm8x3,
    Unorm8x4,
    Snorm8,
    Snorm8x2,
    Snorm8x3,
    Snorm8x4,
    Uint16,
    Uint16x2,
    Uint16x3,
    Uint16x4,
    Sint16,
    Sint16x2,
    Sint16x3,
    Sint16x4,
    Unorm16,
    Unorm16x2,
    Unorm16x3,
    Unorm16x4,
    Snorm16,
    Snorm16x2,
    Snorm16x3,
    Snorm16x4,
    Float16,
    Float16x2,
    Float16x3,
    Float16x4,
    Float32,
    Float32x2,
    Float32x3,
    Float32x4,
    Uint32,
    Uint32x2,
    Uint32x3,
    Uint32x4,
    Sint32,
    Sint32x2,
    Sint32x3,
    Sint32x4,
    Float64,
    Float64x2,
    Float64x3,
    Float64x4,
    Unorm10_10_10_2,
    Unorm8x4Bgra,
};

} // namespace sge

#endif // SGE_RENDERER_VERTEX_FORMAT_HPP_