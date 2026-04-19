#ifndef _SGE_TYPES_TEXTURE_HPP_
#define _SGE_TYPES_TEXTURE_HPP_

#pragma once

#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/Texture.h>
#include <glm/glm.hpp>
#include <SGE/renderer/resource.hpp>
#include <utility>

#include "../types/sampler.hpp"

namespace sge {

namespace TextureSampler {
    enum : uint8_t {
        Linear = 0,
        LinearMips,
        Nearest,
        NearestMips,
    };

    [[nodiscard]]
    inline constexpr uint8_t DisableMips(uint8_t sampler) noexcept {
        switch (sampler) {
            case LinearMips: return Linear;
            case NearestMips: return Nearest;
            default: return sampler;
        }
    }

    [[nodiscard]]
    inline constexpr uint8_t EnableMips(uint8_t sampler) noexcept {
        switch (sampler) {
            case Linear: return LinearMips;
            case Nearest: return NearestMips;
            default: return sampler;
        }
    }
};

class Texture {
public:
    constexpr Texture() = default;

    constexpr Texture(int id, Ref<Sampler> sampler, glm::uvec2 size, Ref<LLGL::Texture> internal) :
        m_internal(std::move(internal)),
        m_sampler(std::move(sampler)),
        m_size(size),
        m_id(id) {}

    [[nodiscard]] inline int id() const { return m_id; }
    [[nodiscard]] inline const Ref<Sampler>& sampler() const { return m_sampler; }
    [[nodiscard]] inline glm::uvec2 size() const { return m_size; }
    [[nodiscard]] inline uint32_t width() const { return m_size.x; }
    [[nodiscard]] inline uint32_t height() const { return m_size.y; }
    [[nodiscard]] inline const Ref<LLGL::Texture>& internal() const { return m_internal; }

    [[nodiscard]] inline bool is_valid() const { return m_id >= 0 && m_internal.IsValid(); }

    inline operator LLGL::Resource*() const { return m_internal.Get(); }
    inline operator LLGL::Texture*() const { return m_internal.Get(); }
    inline operator LLGL::Texture&() const { return *m_internal; }
    inline operator LLGL::ResourceViewDescriptor() const { return m_internal.Get(); }

private:
    Ref<LLGL::Texture> m_internal = nullptr;
    Ref<Sampler> m_sampler = nullptr;
    glm::uvec2 m_size = glm::uvec2(0, 0);
    int m_id = -1;
};

}

#endif