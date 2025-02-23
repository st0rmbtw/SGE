#ifndef UTILS_HPP
#define UTILS_HPP

#pragma once

#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <LLGL/LLGL.h>
#include <glm/glm.hpp>

inline int rand_range(int from, int to) {
    return rand() % (to + 1 - from) + from;
}

inline float rand_range(float from, float to) {
    const float scale = rand() / (float) RAND_MAX;
    return from + scale * (to - from); 
}

#endif