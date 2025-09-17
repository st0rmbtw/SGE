#ifndef _SGE_UTILS_GRADIENT_HPP_
#define _SGE_UTILS_GRADIENT_HPP_

#include <SGE/types/color.hpp>

namespace sge {

struct GradientKey {
    LinearRgba color;
    float position;
};

template <size_t SIZE>
inline LinearRgba GradientEvaluate(const GradientKey (&keys)[SIZE], float position) {
    if (SIZE == 0)
        return LinearRgba::black();

    if (position <= keys[0].position)
        return keys[0].color;

    if (position >= keys[SIZE - 1].position)
        return keys[SIZE - 1].color;

    for (size_t i = 0; i < SIZE - 1; ++i) {
        if (position >= keys[i].position && position <= keys[i + 1].position) {
            float t = (position - keys[i].position) / (keys[i + 1].position - keys[i].position);
            return keys[i].color.lerp(keys[i + 1].color, t);
        }
    }

    return LinearRgba::black();
}

}

#endif