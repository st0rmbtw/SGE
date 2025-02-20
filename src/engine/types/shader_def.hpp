#ifndef _ENGINE_TYPES_SHADER_DEF_HPP_
#define _ENGINE_TYPES_SHADER_DEF_HPP_

#include <string>

struct ShaderDef {
    std::string name;
    std::string value;
    
    ShaderDef(std::string name, std::string value) :
        name(std::move(name)),
        value(std::move(value)) {}
};

#endif