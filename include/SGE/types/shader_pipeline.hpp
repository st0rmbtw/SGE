#ifndef _SGE_TYPES_SHADER_PIPELINE_HPP_
#define _SGE_TYPES_SHADER_PIPELINE_HPP_

#pragma once

#include <SGE/renderer/context.hpp>
#include <LLGL/Shader.h>

namespace sge {

struct ShaderPipeline {
    Ref<LLGL::Shader> vs = nullptr; // Vertex shader
    Ref<LLGL::Shader> hs = nullptr; // Hull shader (aka. tessellation control shader)
    Ref<LLGL::Shader> ds = nullptr; // Domain shader (aka. tessellation evaluation shader)
    Ref<LLGL::Shader> gs = nullptr; // Geometry shader
    Ref<LLGL::Shader> ps = nullptr; // Pixel shader (aka. fragment shader)
    Ref<LLGL::Shader> cs = nullptr; // Compute shader
};

}

#endif