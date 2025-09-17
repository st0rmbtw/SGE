#ifndef _SGE_TYPES_SHADER_DEF_HPP_
#define _SGE_TYPES_SHADER_DEF_HPP_

#include <string>

namespace sge {

struct ShaderDef {
    std::string name;
    std::string value;

    ShaderDef(std::string name, std::string value) noexcept :
        name(std::move(name)),
        value(std::move(value)) {}
};

}

#endif