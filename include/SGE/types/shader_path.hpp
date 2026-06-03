#ifndef _SGE_TYPES_SHADER_PATH_HPP_
#define _SGE_TYPES_SHADER_PATH_HPP_

#include "shader_type.hpp"

namespace sge {

struct ShaderPath {
    ShaderType shader_type;
    std::string name;
    std::string func_name;

    ShaderPath(ShaderType shader_type, std::string name, std::string func_name = {}) noexcept :
        shader_type(shader_type),
        name(std::move(name)),
        func_name(std::move(func_name)) {}
};

} // namespace sge

#endif