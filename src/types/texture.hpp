#ifndef TYPES_TEXTURE_HPP
#define TYPES_TEXTURE_HPP

#pragma once

#include <LLGL/Texture.h>
#include <glm/glm.hpp>

namespace TextureSampler {
    enum : uint8_t {
        Linear = 0,
        LinearMips,
        Nearest,
        NearestMips,
    };
};

struct Texture {
    int id = -1;
    int sampler = 0;
    LLGL::Texture* texture = nullptr;
    glm::uvec2 size;
};

#endif