#ifndef TYPES_SHADER_DEF
#define TYPES_SHADER_DEF

#include <string>

struct ShaderDef {
    std::string name;
    std::string value;
    
    ShaderDef(std::string name, std::string value) :
        name(std::move(name)),
        value(std::move(value)) {}
};

#endif