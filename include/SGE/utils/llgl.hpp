#ifndef SGE_UTILS_LLGL_HPP_
#define SGE_UTILS_LLGL_HPP_

#include <utility>
#include <LLGL/RenderSystemChild.h>

namespace sge {

template <typename T> requires std::derived_from<T, LLGL::RenderSystemChild>
class LLGLResource {
public:
    constexpr LLGLResource() noexcept = default;
    constexpr LLGLResource(T* ptr) noexcept : m_ptr(ptr) {}

    LLGLResource(const LLGLResource&) = delete;
    LLGLResource& operator=(const LLGLResource&) = delete;
    
    LLGLResource(LLGLResource&& other) noexcept {
        operator=(std::move(other));
    }

    LLGLResource& operator=(LLGLResource&& other) noexcept {
        m_ptr = other.m_ptr;
        other.m_ptr = nullptr;
        return *this;
    }

    ~LLGLResource() noexcept {
        m_ptr = nullptr;
    }

    [[nodiscard]]
    inline T* get() const noexcept {
        return m_ptr;
    }

    [[nodiscard]]
    inline T& operator*() const noexcept {
        return *m_ptr;
    }

    [[nodiscard]]
    inline T* operator->() const noexcept {
        return m_ptr;
    }

    [[nodiscard]]
    inline operator bool() const noexcept {
        return m_ptr != nullptr;
    }

private:
    T* m_ptr = nullptr;
};

template <typename T>
inline constexpr bool operator==(const LLGLResource<T>& r, std::nullptr_t) noexcept {
    return !r;
}

template <typename T>
inline constexpr bool operator!=(const LLGLResource<T>& r, std::nullptr_t) noexcept {
    return !operator==(r, nullptr);
}

}

#endif