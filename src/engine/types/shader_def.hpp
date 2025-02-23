#ifndef _SGE_TYPES_SHADER_DEF_HPP_
#define _SGE_TYPES_SHADER_DEF_HPP_

#include <string>

#include "../defines.hpp"

_SGE_BEGIN

namespace types {

struct ShaderDef {
    std::string name;
    std::string value;
    
    ShaderDef(std::string name, std::string value) :
        name(std::move(name)),
        value(std::move(value)) {}
};

}

_SGE_END

#endif