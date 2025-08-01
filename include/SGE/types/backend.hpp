#ifndef _SGE_TYPES_BACKEND_HPP_
#define _SGE_TYPES_BACKEND_HPP_

#pragma once

#include <cstdint>
#include "../assert.hpp"
#include "../defines.hpp"

_SGE_BEGIN

class RenderBackend {
public:
    enum Value : uint8_t {
        Vulkan = 0,
        D3D11,
        D3D12,
        Metal,
        OpenGL
    };

    constexpr RenderBackend() noexcept = default;
    constexpr RenderBackend(Value backend) noexcept : m_value(backend) {}

    constexpr operator Value() const noexcept { return m_value; }
    explicit operator bool() const = delete;

    [[nodiscard]]
    inline constexpr const char* ToString() const noexcept {
        switch (m_value) {
            case Value::Vulkan: return "Vulkan";
            case Value::D3D11: return "Direct3D11";
            case Value::D3D12: return "Direct3D12";
            case Value::Metal: return "Metal";
            case Value::OpenGL: return "OpenGL";
            default: SGE_UNREACHABLE();
        };
    }

    [[nodiscard]]
    inline constexpr const char* AssetFolder() const noexcept {
        switch (m_value) {
            case Value::Vulkan: return "assets/shaders/vulkan/";
            case Value::D3D11: return "assets/shaders/d3d11/";
            case Value::D3D12: return "assets/shaders/d3d11/";
            case Value::Metal: return "assets/shaders/metal/";
            case Value::OpenGL: return "assets/shaders/opengl/";
            default: SGE_UNREACHABLE();
        };
    }

    [[nodiscard]]
    inline constexpr bool IsVulkan() const noexcept {
        return m_value == Value::Vulkan;
    }

    [[nodiscard]]
    inline constexpr bool IsD3D11() const noexcept {
        return m_value == Value::D3D11;
    }

    [[nodiscard]]
    inline constexpr bool IsD3D12() const noexcept {
        return m_value == Value::D3D12;
    }

    [[nodiscard]]
    inline constexpr bool IsMetal() const noexcept {
        return m_value == Value::Metal;
    }

    [[nodiscard]]
    inline constexpr bool IsOpenGL() const noexcept {
        return m_value == Value::OpenGL;
    }

    [[nodiscard]]
    inline constexpr bool IsGLSL() const {
        return m_value == Value::OpenGL || m_value == Value::Vulkan;
    }

    [[nodiscard]]
    inline constexpr bool IsHLSL() const {
        return m_value == Value::D3D11 || m_value == Value::D3D12;
    }

private:
    Value m_value = Value::Vulkan;
};

_SGE_END

#endif