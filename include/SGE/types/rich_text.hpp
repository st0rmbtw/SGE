#pragma once

#ifndef _SGE_RICH_TEXT_HPP_
#define _SGE_RICH_TEXT_HPP_

#include <string_view>
#include <string>
#include <glm/vec3.hpp>

#include "color.hpp"

#include "../defines.hpp"

_SGE_BEGIN

struct RichTextSection {
    std::string_view text;
    sge::LinearRgba color;
    float size;
};

template <size_t Size>
struct RichText {
    RichTextSection sections[Size];

    [[nodiscard]]
    constexpr inline size_t size() const noexcept {
        return Size;
    }

    [[nodiscard]]
    constexpr inline const RichTextSection* data() const noexcept {
        return &sections[0];
    }
};

inline RichText<1> rich_text(std::string_view text, float size, sge::LinearRgba color) {
    return RichText<1>{{ {text, color, size} }};
}

inline RichText<1> rich_text(const std::string& text, float size, sge::LinearRgba color) {
    return RichText<1>{{ {std::string_view(text), color, size} }};
}

inline RichText<1> rich_text(const char* text, float size, sge::LinearRgba color) {
    return RichText<1>{{ {text, color, size} }};
}

_SGE_END

#endif
