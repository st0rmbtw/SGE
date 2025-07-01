#ifndef _SGE_MATH_CONSTS_HPP_
#define _SGE_MATH_CONSTS_HPP_

#include <numbers>

#include "../defines.hpp"

_SGE_BEGIN

namespace consts {
    inline constexpr float E = std::numbers::e_v<float>;
    inline constexpr float LOG2_E = std::numbers::log2e_v<float>;
    inline constexpr float LOG10_E = std::numbers::log10e_v<float>;
    inline constexpr float PI = std::numbers::pi_v<float>;
    inline constexpr float INV_PI = std::numbers::inv_pi_v<float>;
    inline constexpr float INV_SQRT_PI = std::numbers::inv_sqrtpi_v<float>;
    inline constexpr float LN_2 = std::numbers::ln2_v<float>;
    inline constexpr float LN_10 = std::numbers::ln10_v<float>;
    inline constexpr float SQRT_2 = std::numbers::sqrt2_v<float>;
    inline constexpr float SQRT_3 = std::numbers::sqrt3_v<float>;
    inline constexpr float INV_SQRT_3 = std::numbers::inv_sqrt3_v<float>;
    inline constexpr float E_GAMMA = std::numbers::egamma_v<float>;
    inline constexpr float PHI = std::numbers::phi_v<float>;
}

_SGE_END

#endif