#ifndef _SGE_MATH_MATH_HPP_
#define _SGE_MATH_MATH_HPP_

#include <cmath>

#include "../defines.hpp"

_SGE_BEGIN

namespace math {

inline float rem_euclid(float lhs, float rhs) {
    float r = fmodf(lhs, rhs);
    return r < 0.0f ? r + std::abs(rhs) : r;
}

}

_SGE_END

#endif