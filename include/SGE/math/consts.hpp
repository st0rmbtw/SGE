#ifndef _SGE_MATH_CONSTS_HPP_
#define _SGE_MATH_CONSTS_HPP_

#include <numbers>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace sge::consts {
    inline constexpr float E = std::numbers::e_v<float>;
    inline constexpr float LOG2_E = std::numbers::log2e_v<float>;
    inline constexpr float LOG10_E = std::numbers::log10e_v<float>;
    inline constexpr float PI = std::numbers::pi_v<float>;
    inline constexpr float TAU = std::numbers::pi_v<float> * 2.0f;
    inline constexpr float FRAC_PI_2 = PI / 2.0f;
    inline constexpr float FRAC_PI_4 = PI / 4.0f;
    inline constexpr float FRAC_PI_6 = PI / 6.0f;
    inline constexpr float FRAC_PI_8 = PI / 8.0f;
    inline constexpr float INV_PI = std::numbers::inv_pi_v<float>;
    inline constexpr float INV_SQRT_PI = std::numbers::inv_sqrtpi_v<float>;
    inline constexpr float LN_2 = std::numbers::ln2_v<float>;
    inline constexpr float LN_10 = std::numbers::ln10_v<float>;
    inline constexpr float SQRT_2 = std::numbers::sqrt2_v<float>;
    inline constexpr float SQRT_3 = std::numbers::sqrt3_v<float>;
    inline constexpr float INV_SQRT_3 = std::numbers::inv_sqrt3_v<float>;
    inline constexpr float E_GAMMA = std::numbers::egamma_v<float>;
    inline constexpr float PHI = std::numbers::phi_v<float>;

    namespace vec2 {
        inline glm::vec2 ONE = glm::vec2(1.f);
        inline glm::vec2 ZERO = glm::vec2(0.f);
        
        inline glm::vec2 X = glm::vec2(1.f, 0.f);
        inline glm::vec2 Y = glm::vec2(0.f, 1.f);

        inline glm::vec2 NEG_X = glm::vec2(-1.f, 0.f);
        inline glm::vec2 NEG_Y = glm::vec2(0.f, -1.f);
    } // namespace vec2

    namespace vec3 {
        inline glm::vec3 ONE = glm::vec3(1.f);
        inline glm::vec3 ZERO = glm::vec3(0.f);

        inline glm::vec3 X = glm::vec3(1.f, 0.f, 0.f);
        inline glm::vec3 Y = glm::vec3(0.f, 1.f, 0.f);
        inline glm::vec3 Z = glm::vec3(0.f, 0.f, 1.f);

        inline glm::vec3 NEG_X = glm::vec3(-1.f, 0.f, 0.f);
        inline glm::vec3 NEG_Y = glm::vec3(0.f, -1.f, 0.f);
        inline glm::vec3 NEG_Z = glm::vec3(0.f, 0.f, -1.f);
    } // namespace vec3

    namespace vec4 {
        inline glm::vec4 ONE = glm::vec4(1.0f);
        inline glm::vec4 ZERO = glm::vec4(0.0f);

        inline glm::vec4 X = glm::vec4(1.f, 0.f, 0.f, 0.f);
        inline glm::vec4 Y = glm::vec4(0.f, 1.f, 0.f, 0.f);
        inline glm::vec4 Z = glm::vec4(0.f, 0.f, 1.f, 0.f);
        inline glm::vec4 W = glm::vec4(0.f, 0.f, 0.f, 1.f);

        inline glm::vec4 NEG_X = glm::vec4(-1.f,  0.f,  0.f,  0.f);
        inline glm::vec4 NEG_Y = glm::vec4( 0.f, -1.f,  0.f,  0.f);
        inline glm::vec4 NEG_Z = glm::vec4( 0.f,  0.f, -1.f,  0.f);
        inline glm::vec4 NEG_W = glm::vec4( 0.f,  0.f,  0.f, -1.f);
    } // namespace vec4

} // namespace sge::consts

#endif