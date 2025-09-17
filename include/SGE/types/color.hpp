#ifndef _SGE_TYPES_COLOR_HPP_
#define _SGE_TYPES_COLOR_HPP_

#pragma once

#include <type_traits>
#include <algorithm>

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

#include "../math/math.hpp"

namespace sge {

struct LinearRgba;
struct Hsla;
struct Srgba;

namespace detail {
    inline float lerp_hue(float a, float b, float t) {
        float diff = sge::rem_euclid(b - a + 180.0f, 360.0f) - 180.0f;
        return sge::rem_euclid(a + diff * t, 360.0f);
    }

    inline LinearRgba hsla_to_linear_rgba(const Hsla& hlsa);
    inline Hsla linear_rgba_to_hlsa(const LinearRgba& rgba);

    inline LinearRgba srgba_to_linear_rgba(const Srgba& srgba);
    inline Srgba linear_rgba_to_srgba(const LinearRgba& rgba);

    /*
    * Converts an HUE to r, g or b.
    * returns float in the set [0, 1].
    */
    inline float hue2rgb(float p, float q, float t) {
        if (t < 0) 
            t += 1;
        if (t > 1) 
            t -= 1;
        if (t < 1./6) 
            return p + (q - p) * 6 * t;
        if (t < 1./2) 
            return q;
        if (t < 2./3)   
            return p + (q - p) * (2./3 - t) * 6;
            
        return p;
    }
}

struct LinearRgba {
    LinearRgba() = default;

    explicit constexpr LinearRgba(glm::vec4 color) : r(color.r), g(color.g), b(color.b), a(color.a) {}

    explicit constexpr LinearRgba(float x) : r(x), g(x), b(x), a(x) {}
    explicit constexpr LinearRgba(float x, float alpha) : r(x), g(x), b(x), a(alpha) {}

    template <typename T>
    explicit constexpr LinearRgba(T t_r, T t_g, T t_b, T t_a = 1.0f) requires (std::is_floating_point_v<T>) : r(t_r), g(t_g), b(t_b), a(t_a) {}

    template <typename T>
    explicit constexpr LinearRgba(T t_r, T t_g, T t_b, T t_a = 0xFF) requires (std::is_integral_v<T>) :
        r(t_r / 255.0f),
        g(t_g / 255.0f),
        b(t_b / 255.0f),
        a(t_a / 255.0f) {}

    template <typename T1, typename T2>
    explicit constexpr LinearRgba(T1 t_r, T1 t_g, T1 t_b, T2 t_a = 1.0f) requires (std::is_integral_v<T1> && std::is_floating_point_v<T2>) :
        r(t_r / 255.0f),
        g(t_g / 255.0f),
        b(t_b / 255.0f),
        a(t_a) {}

    explicit constexpr LinearRgba(glm::vec3 color, float t_a = 1.0f) : r(color.r), g(color.g), b(color.b), a(t_a) {}

    static constexpr LinearRgba white() noexcept {
        return LinearRgba(1.0f, 1.0f, 1.0f, 1.0f);
    }

    static constexpr LinearRgba black() noexcept {
        return LinearRgba(0.0f, 0.0f, 0.0f, 1.0f);
    }

    static constexpr LinearRgba transparent() noexcept {
        return LinearRgba(0.0f, 0.0f, 0.0f, 0.0f);
    }

    static constexpr LinearRgba red() noexcept {
        return LinearRgba(1.0f, 0.0f, 0.0f, 1.0f);
    }

    static constexpr LinearRgba green() noexcept {
        return LinearRgba(0.0f, 1.0f, 0.0f, 1.0f);
    }

    static constexpr LinearRgba blue() noexcept {
        return LinearRgba(0.0f, 0.0f, 1.0f, 1.0f);
    }

    [[nodiscard]]
    float luminance() const noexcept {
        return r * 0.2126 + g * 0.7152 + b * 0.0722;
    }

    [[nodiscard]]
    LinearRgba darker(float amount) const noexcept {
        LinearRgba result = *this;
        result.adjust_brightness(-amount);
        return result;
    }

    [[nodiscard]]
    LinearRgba lighter(float amount) const noexcept {
        LinearRgba result = *this;
        result.adjust_brightness(amount);
        return result;
    }

    [[nodiscard]]
    LinearRgba lerp(const LinearRgba& other, float t) const noexcept {
        const float n_t = 1.0f - t;
        return LinearRgba(
            r * n_t + other.r * t,
            g * n_t + other.g * t,
            b * n_t + other.b * t,
            a * n_t + other.a * t
        );
    }

    void adjust_brightness(float amount) {
        const float l = luminance();
        const float l_target = std::clamp(l + amount, 0.0f, 1.0f);
        if (l_target < l) {
            const float adjustment = (l - l_target) / l;
            *this = lerp(LinearRgba(0.0f, 0.0f, 0.0f, a), adjustment);
        } else if (l_target > l) {
            const float adjustment = (l_target - l) / (1.0f - l);
            *this = lerp(LinearRgba(1.0f, 1.0f, 1.0f, a), adjustment);
        }
    }

    [[nodiscard]]
    inline Hsla to_hsla() const;

    [[nodiscard]]
    inline Srgba to_srgba() const;

    [[nodiscard]]
    glm::vec4 to_vec4() const noexcept {
        return glm::vec4(r, g, b, a);
    }

    [[nodiscard]]
    glm::vec3 to_vec3() const noexcept {
        return glm::vec3(r, g, b);
    }

    constexpr LinearRgba operator*(const LinearRgba c) const noexcept {
        return LinearRgba(r * c.r, g * c.g, b * c.b, a * c.a);
    }

    constexpr LinearRgba operator+(const LinearRgba c) const noexcept {
        return LinearRgba(r + c.r, g + c.g, b + c.b, a + c.a);
    }

    constexpr LinearRgba operator-(const LinearRgba c) const noexcept {
        return LinearRgba(r - c.r, g - c.g, b - c.b, a - c.a);
    }

    constexpr LinearRgba operator*(const float x) const noexcept {
        return LinearRgba(r * x, g * x, b * x, a * x);
    }

    constexpr LinearRgba operator+(const float x) const noexcept {
        return LinearRgba(r + x, g + x, b + x, a + x);
    }

    constexpr LinearRgba operator-(const float x) const noexcept {
        return LinearRgba(r - x, g - x, b - x, a - x);
    }

    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct Srgba {
    Srgba() = default;

    explicit constexpr Srgba(const glm::vec4& color) : red(color.r), green(color.g), blue(color.b), alpha(color.a) {}

    explicit constexpr Srgba(float x) : red(x), green(x), blue(x), alpha(x) {}
    explicit constexpr Srgba(float x, float alpha) : red(x), green(x), blue(x), alpha(alpha) {}

    template <typename T>
    explicit constexpr Srgba(T t_r, T t_g, T t_b, T t_a = 1.0f) requires (std::is_floating_point_v<T>) : red(t_r), green(t_g), blue(t_b), alpha(t_a) {}

    template <typename T>
    explicit constexpr Srgba(T t_r, T t_g, T t_b, T t_a = 0xFF) requires (std::is_integral_v<T>) :
        red(t_r / 255.0f),
        green(t_g / 255.0f),
        blue(t_b / 255.0f),
        alpha(t_a / 255.0f) {}

    template <typename T1, typename T2>
    explicit constexpr Srgba(T1 t_r, T1 t_g, T1 t_b, T2 t_a = 1.0f) requires (std::is_integral_v<T1> && std::is_floating_point_v<T2>) :
        red(t_r / 255.0f),
        green(t_g / 255.0f),
        blue(t_b / 255.0f),
        alpha(t_a) {}

    explicit constexpr Srgba(const glm::vec3& color, float t_a = 1.0f) : red(color.r), green(color.g), blue(color.b), alpha(t_a) {}

    static constexpr Srgba white() {
        return Srgba(1.0f, 1.0f);
    }

    static constexpr Srgba black() {
        return Srgba(0.0f, 1.0f);
    }

    [[nodiscard]]
    float luminance() const {
        return red * 0.2126 + green * 0.7152 + blue * 0.0722;
    }

    [[nodiscard]]
    Srgba darker(float amount) const {
        Srgba result = *this;
        result.adjust_brightness(-amount);
        return result;
    }

    [[nodiscard]]
    Srgba lighter(float amount) const {
        Srgba result = *this;
        result.adjust_brightness(amount);
        return result;
    }

    // https://en.wikipedia.org/wiki/Gamma_correction
    [[nodiscard]]
    static float correct_gamma(float value) {
        if (value <= 0.0f) return value;

        if (value <= 0.04045f) {
            return value / 12.92f; // linear falloff in dark values
        } else {
            return std::powf((value + 0.055f) / 1.055f, 2.4f); // gamma curve in other area
        }
    }

    // https://en.wikipedia.org/wiki/Gamma_correction
    static float correct_gamma_inverse(float value) {
        if (value <= 0.0f) return value;

        if (value <= 0.0031308f) {
            return value * 12.92f; // linear falloff in dark values
        } else {
            return (1.055f * std::powf(value, 1.0f / 2.4f)) - 0.055f; // gamma curve in other area
        }
    }

    [[nodiscard]]
    Srgba lerp(const Srgba& other, float t) const {
        const float n_t = 1.0f - t;
        return Srgba(
            red * n_t + other.red * t,
            green * n_t + other.green * t,
            blue * n_t + other.blue * t,
            alpha * n_t + other.alpha * t
        );
    }

    void adjust_brightness(float amount) {
        const float l = luminance();
        const float l_target = std::clamp(l + amount, 0.0f, 1.0f);
        if (l_target < l) {
            const float adjustment = (l - l_target) / l;
            *this = lerp(Srgba(0.0f, 0.0f, 0.0f, alpha), adjustment);
        } else if (l_target > l) {
            const float adjustment = (l_target - l) / (1.0f - l);
            *this = lerp(Srgba(1.0f, 1.0f, 1.0f, alpha), adjustment);
        }
    }

    [[nodiscard]] inline Hsla to_hsla() const;
    [[nodiscard]] inline LinearRgba to_linear_rgba() const;

    inline operator LinearRgba() const { return to_linear_rgba(); }

    [[nodiscard]]
    glm::vec4 to_vec4() const {
        return glm::vec4(red, green, blue, alpha);
    }
    
    [[nodiscard]]
    glm::vec3 to_vec3() const {
        return glm::vec3(red, green, blue);
    }

    [[nodiscard]]
    constexpr Srgba operator*(const Srgba c) const {
        return Srgba(red * c.red, green * c.green, blue * c.blue, alpha * c.alpha);
    }

    [[nodiscard]]
    constexpr Srgba operator+(const Srgba c) const {
        return Srgba(red + c.red, green + c.green, blue + c.blue, alpha + c.alpha);
    }

    [[nodiscard]]
    constexpr Srgba operator-(const Srgba c) const {
        return Srgba(red - c.red, green - c.green, blue - c.blue, alpha - c.alpha);
    }

    [[nodiscard]]
    constexpr Srgba operator*(const float x) const {
        return Srgba(red * x, green * x, blue * x, alpha * x);
    }

    [[nodiscard]]
    constexpr Srgba operator+(const float x) const {
        return Srgba(red + x, green + x, blue + x, alpha + x);
    }

    [[nodiscard]]
    constexpr Srgba operator-(const float x) const {
        return Srgba(red - x, green - x, blue - x, alpha - x);
    }

    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
    float alpha = 1.0f;
};

struct Hsla {
    constexpr Hsla() = default;

    explicit constexpr Hsla(float t_hue, float t_saturation, float t_lightness, float t_alpha = 1.0f) :
        hue(t_hue),
        saturation(t_saturation),
        lightness(t_lightness),
        alpha(t_alpha) {}

    static constexpr Hsla white() {
        return Hsla(0.0f, 0.0f, 100.0f, 1.0);
    }

    static constexpr Hsla black() {
        return Hsla(0.0f, 0.0f, 0.0f, 1.0);
    }

    [[nodiscard]]
    float luminance() const {
        return lightness;
    }

    [[nodiscard]]
    Hsla lighter(float amount) const {
        const float l = std::min(lightness + amount, 1.0f);
        return Hsla(hue, saturation, l, alpha);
    }

    [[nodiscard]]
    Hsla darker(float amount) const {
        const float l = std::clamp(lightness - amount, 0.0f, 1.0f);
        return Hsla(hue, saturation, l, alpha);
    }

    [[nodiscard]]
    Hsla lerp(const Hsla& other, float t) const {
        const float n_t = 1.0f - t;
        return Hsla(
            detail::lerp_hue(hue, other.hue, t),
            saturation * n_t + other.saturation * t,
            lightness * n_t + other.lightness * t,
            alpha * n_t + other.alpha * t
        );
    }

    [[nodiscard]]
    inline LinearRgba to_linear_rgba() const;

    inline operator LinearRgba() const { return to_linear_rgba(); }

    float hue = 0.0f;
    float saturation = 0.0f;
    float lightness = 1.0f;
    float alpha = 1.0f;
};

inline Hsla LinearRgba::to_hsla() const {
   return detail::linear_rgba_to_hlsa(*this);
}

inline LinearRgba Hsla::to_linear_rgba() const {
    return detail::hsla_to_linear_rgba(*this);
}

inline LinearRgba Srgba::to_linear_rgba() const {
    return detail::srgba_to_linear_rgba(*this);
}

inline Srgba LinearRgba::to_srgba() const {
    return detail::linear_rgba_to_srgba(*this);
}

inline LinearRgba detail::hsla_to_linear_rgba(const Hsla& hlsa) {
    LinearRgba result;
    result.a = hlsa.alpha;

    if (hlsa.saturation == 0.0f) {
        result.r = result.g = result.b = hlsa.lightness; // achromatic
    } else {
        const float q = hlsa.lightness < 0.5f ? hlsa.lightness * (1.0f + hlsa.saturation) : hlsa.lightness + hlsa.saturation - hlsa.lightness * hlsa.saturation;
        const float p = 2.0f * hlsa.lightness - q;
        const float hue = hlsa.hue / 360.0f;
        result.r = hue2rgb(p, q, hue + 1.0f / 3.0f);
        result.g = hue2rgb(p, q, hue);
        result.b = hue2rgb(p, q, hue - 1.0f / 3.0f);
    }
    return result;
}

inline Hsla detail::linear_rgba_to_hlsa(const LinearRgba& rgba) {
    Hsla result;
    
    float max = std::max(std::max(rgba.r, rgba.g), rgba.b);
    float min = std::min(std::min(rgba.r, rgba.g), rgba.b);
    
    result.hue = result.saturation = result.lightness = (max + min) / 2;

    if (max == min) {
        result.hue = result.saturation = 0.0f; // achromatic
    } else {
        float d = max - min;
        result.saturation = (result.lightness > 0.5f) ? d / (2.0f - max - min) : d / (max + min);
        
        if (max == rgba.r) {
            result.hue = (rgba.g - rgba.b) / d + (rgba.g < rgba.b ? 6.0f : 0);
        } else if (max == rgba.g) {
            result.hue = (rgba.b - rgba.r) / d + 2.0f;
        } else if (max == rgba.b) {
            result.hue = (rgba.r - rgba.g) / d + 4.0f;
        }
        
        result.hue /= 6.0f;
    }

    return result;
}

inline LinearRgba detail::srgba_to_linear_rgba(const Srgba& srgba) {
    return LinearRgba(
        Srgba::correct_gamma(srgba.red),
        Srgba::correct_gamma(srgba.blue),
        Srgba::correct_gamma(srgba.green),
        srgba.alpha
    );
}

inline Srgba detail::linear_rgba_to_srgba(const LinearRgba& rgba) {
    return Srgba(
        Srgba::correct_gamma_inverse(rgba.r),
        Srgba::correct_gamma_inverse(rgba.b),
        Srgba::correct_gamma_inverse(rgba.g),
        rgba.a
    );
}

}

#endif