#ifndef _SGE_TYPES_SHADER_TYPE_HPP_
#define _SGE_TYPES_SHADER_TYPE_HPP_

#pragma once

#include <cstdint>
#include <LLGL/ShaderFlags.h>

#include "backend.hpp"

#include "../assert.hpp"

namespace sge {

class ShaderType {
public:
    enum Value : uint8_t {
        Vertex = 0,
        Fragment,
        Geometry,
        Compute
    };

    ShaderType() = default;
    constexpr ShaderType(Value backend) : m_value(backend) {}

    constexpr operator Value() const { return m_value; }
    explicit operator bool() const = delete;

    [[nodiscard]]
    inline constexpr LLGL::ShaderType ToLLGLType() const noexcept {
        switch (m_value) {
            case Value::Vertex: return LLGL::ShaderType::Vertex;
            case Value::Fragment: return LLGL::ShaderType::Fragment;
            case Value::Geometry: return LLGL::ShaderType::Geometry;
            case Value::Compute: return LLGL::ShaderType::Compute;
            default: SGE_UNREACHABLE();
        };
    }

    [[nodiscard]]
    inline constexpr const char* EntryPoint(RenderBackend backend) const noexcept {
        if (backend.IsOpenGL() || backend.IsVulkan()) return nullptr;

        switch (m_value) {
            case Value::Vertex: return "VS";
            case Value::Fragment: return "PS";
            case Value::Geometry: return "GS";
            default: return nullptr;
        };
    }

    [[nodiscard]]
    inline constexpr const char* Profile(RenderBackend backend) const noexcept {
        switch (backend) {
        case RenderBackend::OpenGL:
        case RenderBackend::Vulkan: return nullptr;

        case RenderBackend::D3D11: switch (m_value) {
            case Value::Vertex: return "vs_5_0";
            case Value::Fragment: return "ps_5_0";
            case Value::Geometry: return "gs_5_0";
            case Value::Compute: return "cs_5_0";
            default: return nullptr;
        };
        break;

        case RenderBackend::D3D12: switch (m_value) {
            case Value::Vertex: return "vs_5_0";
            case Value::Fragment: return "ps_5_0";
            case Value::Geometry: return "gs_5_0";
            case Value::Compute: return "cs_5_0";
            default: return nullptr;
        };
        break;

        case RenderBackend::Metal: return "2.2";
        default: SGE_UNREACHABLE();
        }
    }

    [[nodiscard]]
    inline constexpr const char* Stage() const noexcept {
        switch (m_value) {
            case Value::Vertex:   return "vert";
            case Value::Fragment: return "frag";
            case Value::Geometry: return "geom";
            case Value::Compute:  return "comp";
            default: return nullptr;
        };
    }

    [[nodiscard]]
    inline constexpr const char* FileExtension(RenderBackend backend) const noexcept {
        switch (backend) {
            case RenderBackend::D3D11:
            case RenderBackend::D3D12: return ".hlsl";
            case RenderBackend::Metal: return ".metal";
            case RenderBackend::Vulkan: return ".spv";
            case RenderBackend::OpenGL: return ".glsl";
            default: return nullptr;
        }
    }

    [[nodiscard]]
    inline constexpr bool IsVertex() const noexcept {
        return m_value == Value::Vertex;
    }

    [[nodiscard]]
    inline constexpr bool IsFragment() const noexcept {
        return m_value == Value::Fragment;
    }

    [[nodiscard]]
    inline constexpr bool IsGeometry() const noexcept {
        return m_value == Value::Geometry;
    }

    [[nodiscard]]
    inline constexpr bool IsCompute() const noexcept {
        return m_value == Value::Compute;
    }

private:
    Value m_value = Value::Vertex;
};

}

#endif