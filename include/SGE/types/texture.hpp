#ifndef _SGE_TYPES_TEXTURE_HPP_
#define _SGE_TYPES_TEXTURE_HPP_

#pragma once

#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/Texture.h>
#include <glm/glm.hpp>

#include "../types/sampler.hpp"

#include "../defines.hpp"

_SGE_BEGIN

namespace TextureSampler {
    enum : uint8_t {
        Linear = 0,
        LinearMips,
        Nearest,
        NearestMips,
    };
};

class Texture {
public:
    Texture() = default;

    Texture(int id, Sampler sampler, glm::uvec2 size, LLGL::Texture* internal) :
        m_sampler(sampler),
        m_size(size),
        m_internal(internal),
        m_id(id) {}

    [[nodiscard]] inline int id() const { return m_id; }
    [[nodiscard]] inline const Sampler& sampler() const { return m_sampler; }
    [[nodiscard]] inline glm::uvec2 size() const { return m_size; }
    [[nodiscard]] inline uint32_t width() const { return m_size.x; }
    [[nodiscard]] inline uint32_t height() const { return m_size.y; }
    [[nodiscard]] inline LLGL::Texture* internal() const { return m_internal; }

    [[nodiscard]] inline bool is_valid() const { return m_id >= 0 && m_internal != nullptr; }

    inline operator LLGL::Resource*() const { return m_internal; }
    inline operator LLGL::Texture*() const { return m_internal; }
    inline operator LLGL::Texture&() const { return *m_internal; }
    inline operator LLGL::ResourceViewDescriptor() const { return m_internal; }

private:
    Sampler m_sampler;
    glm::uvec2 m_size = glm::uvec2(0, 0);
    LLGL::Texture* m_internal = nullptr;
    int m_id = -1;
};

_SGE_END

#endif