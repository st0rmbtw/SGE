#ifndef _SGE_TYPES_SHADER_PIPELINE_HPP_
#define _SGE_TYPES_SHADER_PIPELINE_HPP_

#pragma once

#include <LLGL/Shader.h>
#include <LLGL/RenderSystem.h>

#include "../defines.hpp"

_SGE_BEGIN

struct ShaderPipeline {
    LLGL::Shader* vs = nullptr; // Vertex shader
    LLGL::Shader* hs = nullptr; // Hull shader (aka. tessellation control shader)
    LLGL::Shader* ds = nullptr; // Domain shader (aka. tessellation evaluation shader)
    LLGL::Shader* gs = nullptr; // Geometry shader
    LLGL::Shader* ps = nullptr; // Pixel shader (aka. fragment shader)
    LLGL::Shader* cs = nullptr; // Compute shader
    
    void Unload(const LLGL::RenderSystemPtr& context) const {
        if (vs != nullptr) context->Release(*vs);
        if (hs != nullptr) context->Release(*hs);
        if (ds != nullptr) context->Release(*ds);
        if (gs != nullptr) context->Release(*gs);
        if (ps != nullptr) context->Release(*ps);
        if (cs != nullptr) context->Release(*cs);
    }
};

_SGE_END

#endif