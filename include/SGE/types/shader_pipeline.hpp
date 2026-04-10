#ifndef _SGE_TYPES_SHADER_PIPELINE_HPP_
#define _SGE_TYPES_SHADER_PIPELINE_HPP_

#pragma once

#include <SGE/renderer/context.hpp>
#include <LLGL/Shader.h>

namespace sge {

struct ShaderPipeline {
    LLGL::Shader* vs = nullptr; // Vertex shader
    LLGL::Shader* hs = nullptr; // Hull shader (aka. tessellation control shader)
    LLGL::Shader* ds = nullptr; // Domain shader (aka. tessellation evaluation shader)
    LLGL::Shader* gs = nullptr; // Geometry shader
    LLGL::Shader* ps = nullptr; // Pixel shader (aka. fragment shader)
    LLGL::Shader* cs = nullptr; // Compute shader
    
    void Unload(sge::RenderContext& context) const {
        if (vs != nullptr)
            context.DeleteResource(*vs);
        if (hs != nullptr)
            context.DeleteResource(*hs);
        if (ds != nullptr)
            context.DeleteResource(*ds);
        if (gs != nullptr)
            context.DeleteResource(*gs);
        if (ps != nullptr)
            context.DeleteResource(*ps);
        if (cs != nullptr)
            context.DeleteResource(*cs);
    }
};

}

#endif