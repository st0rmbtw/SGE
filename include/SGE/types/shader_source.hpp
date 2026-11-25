#ifndef SGE_TYPES_SHADER_SOURCE_HPP_
#define SGE_TYPES_SHADER_SOURCE_HPP_

#include <span>
#include <LLGL/ShaderFlags.h>

namespace sge {

struct ShaderSource {
    std::span<const std::byte> data;
    LLGL::ShaderSourceType type;
};

} // namespace sge

#endif