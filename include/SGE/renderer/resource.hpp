#ifndef SGE_RENDERER_RESOURCE_HPP_
#define SGE_RENDERER_RESOURCE_HPP_

#include "SGE/log.hpp"
#include <LLGL/RenderSystemChild.h>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace sge {

class RenderContext;

class RefCounted {
public:
    inline void IncRefCount() const {
        ++m_ref_count;
    };

    inline void DecRefCount() const {
        --m_ref_count;
    };

    [[nodiscard]]
    inline uint32_t GetRefCount() const noexcept {
        return m_ref_count;
    }

private:
    mutable uint32_t m_ref_count = 0;
};


class LLGLResourceRC : public RefCounted {
public:
    explicit LLGLResourceRC(std::shared_ptr<RenderContext> context, LLGL::RenderSystemChild* data) :
        m_render_context(std::move(context)),
        m_data(data),
        m_id(s_id++)
    {
        SGE_LOG_DEBUG("Created: {}", m_id);
    }
    ~LLGLResourceRC();

    inline LLGL::RenderSystemChild* Get() noexcept {
        return m_data;
    }

private:
    std::shared_ptr<RenderContext> m_render_context = nullptr;
    LLGL::RenderSystemChild* m_data = nullptr;
    uint32_t m_id = 0;
    static uint32_t s_id;
};

class LLGLResource {
public:
    LLGLResource() = default;

    explicit LLGLResource(std::shared_ptr<RenderContext> context, LLGL::RenderSystemChild* data) :
        m_render_context(std::move(context)),
        m_data(data)
    {}

    ~LLGLResource();

    LLGLResource(const LLGLResource& other) = default;
    LLGLResource& operator=(const LLGLResource& other) = default;

    LLGLResource(LLGLResource&& other) noexcept {
        operator=(std::move(other));
    }

    LLGLResource& operator=(LLGLResource&& other) noexcept {
        m_render_context = std::move(other.m_render_context);
        m_data = other.m_data;
        other.m_data = nullptr;
        return *this;
    }

    [[nodiscard]]
    inline LLGL::RenderSystemChild* Get() const noexcept {
        return m_data;
    }

    [[nodiscard]]
    bool IsValid() const noexcept {
        return m_data != nullptr;
    }

private:
    std::shared_ptr<RenderContext> m_render_context = nullptr;
    LLGL::RenderSystemChild* m_data = nullptr;
};

template <typename T>
class Ref;

template <typename T> requires std::derived_from<T, LLGL::RenderSystemChild>
class RefWeak;

template <typename T> requires std::derived_from<T, RefCounted>
class Ref<T> {
public:
    Ref() = default;
    Ref(std::nullptr_t) {}

    Ref(T* data) : m_data(data) {
        IncrementRef();
    }

    ~Ref() {
        DecrementRef();
    }

    Ref(Ref&& other) noexcept {
        operator=(std::move(other));
    }

    Ref(const Ref& other) {
        operator=(other);
    }

    Ref& operator=(Ref&& other) noexcept {
        DecrementRef();
        m_data = other.m_data;
        other.m_data = nullptr;
        return *this;
    }

    Ref& operator=(const Ref& other) {
        other.IncrementRef();
        DecrementRef();
        m_data = other.m_data;
        return *this;
    }
    
    template <typename... Args>
    static Ref& Create(Args... args) {
        return Ref(new T(std::forward<Args>(args)...));
    }

    T* Get() const noexcept {
        return m_data;
    }

    [[nodiscard]]
    inline bool IsValid() const noexcept {
        return m_data != nullptr;
    }

    operator bool() const noexcept {
        return IsValid();
    }

    T* operator->() const noexcept {
        return Get();
    }

    T& operator*() const noexcept {
        return *Get();
    }

private:
    void IncrementRef() const {
        if (m_data) {
            m_data->IncRefCount();
        }
    }

    void DecrementRef() const {
        if (m_data) {
            m_data->DecRefCount();
            if (m_data->GetRefCount() == 0) {
                delete m_data;
            }
        }
    }

private:
    T* m_data = nullptr;
};


template <typename T> requires std::derived_from<T, LLGL::RenderSystemChild>
class Ref<T> {
public:
    Ref() = default;
    Ref(std::nullptr_t) {}

    Ref(std::shared_ptr<RenderContext> context, T* ptr) : 
        m_data(new LLGLResourceRC(std::move(context), ptr))
    {
        IncrementRef();
    }

    ~Ref() {
        DecrementRef();
    }

    Ref(Ref&& other) noexcept {
        operator=(std::move(other));
    }

    Ref(const Ref& other) {
        operator=(other);
    }

    Ref& operator=(Ref&& other) noexcept {
        DecrementRef();
        m_data = other.m_data;
        other.m_data = nullptr;
        return *this;
    }

    Ref& operator=(const Ref& other) {
        other.IncrementRef();
        DecrementRef();
        m_data = other.m_data;
        return *this;
    }

    T* Get() const noexcept {
        if (m_data)
            return static_cast<T*>(m_data->Get());
        else
            return nullptr;
    }

    operator T*() const noexcept {
        return Get();
    }

    [[nodiscard]]
    inline bool IsValid() const noexcept {
        return m_data != nullptr && m_data->Get() != nullptr;
    }

    operator bool() const noexcept {
        return IsValid();
    }

    T* operator->() const noexcept {
        return Get();
    }

    T& operator*() const noexcept {
        return *Get();
    }

private:
    void IncrementRef() const {
        if (m_data) {
            m_data->IncRefCount();
        }
    }

    void DecrementRef() const {
        if (m_data) {
            m_data->DecRefCount();
            if (m_data->GetRefCount() == 0) {
                delete m_data;
            }
        }
    }

private:
    LLGLResourceRC* m_data = nullptr;
};

template <typename T> requires std::derived_from<T, LLGL::RenderSystemChild>
class RefWeak {
public:
    RefWeak() = default;
    RefWeak(std::nullptr_t) {}
    RefWeak(const Ref<T>& ref) : m_data(ref.m_data) {
    }

    T* Get() const noexcept {
        if (m_data)
            return static_cast<T*>(m_data->Get());
        else
            return nullptr;
    }

    [[nodiscard]]
    inline bool IsValid() const noexcept {
        return m_data != nullptr && m_data->Get() != nullptr;
    }

    operator bool() const noexcept {
        return IsValid();
    }

    T* operator->() const noexcept {
        return Get();
    }

    T& operator*() const noexcept {
        return *Get();
    }

private:
    LLGLResource* m_data = nullptr;
};


template <typename T>
class Unique;

template <typename T> requires std::derived_from<T, LLGL::RenderSystemChild>
class Unique<T> {
public:
    Unique() = default;
    Unique(std::nullptr_t) {}
    Unique(std::shared_ptr<RenderContext> context, T* data) :
        m_data(LLGLResource(std::move(context), data))
    {
    }

    Unique(const Unique&) = delete;
    Unique& operator=(const Unique&) = delete;

    Unique(Unique&& other) noexcept {
        operator=(std::move(other));
    }
    Unique& operator=(Unique&& other) noexcept {
        m_data = std::move(other.m_data);
        return *this;
    }

    T* Get() const noexcept {
        if (m_data.IsValid())
            return static_cast<T*>(m_data.Get());
        else
            return nullptr;
    }

    operator T*() const noexcept {
        return Get();
    }

    [[nodiscard]]
    inline bool IsValid() const noexcept {
        return m_data.IsValid();
    }

    operator bool() const noexcept {
        return IsValid();
    }

    T* operator->() const noexcept {
        return Get();
    }

    T& operator*() const noexcept {
        return *Get();
    }

private:
    LLGLResource m_data;
};

template <typename T> requires (!std::derived_from<T, LLGL::RenderSystemChild>)
class Unique<T> {
public:
    Unique() = default;
    Unique(std::nullptr_t) {}
    Unique(T* data) : m_data(data) {}

    ~Unique() {
        delete m_data;
    }

    Unique(const Unique&) = delete;
    Unique& operator=(const Unique&) = delete;

    Unique(Unique&& other) noexcept {
        operator=(std::move(other));
    }
    Unique& operator=(Unique&& other) noexcept {
        m_data = other.m_data;
        other.m_data = nullptr;
        return *this;
    }

    T* Get() const noexcept {
        return m_data;
    }

    operator T*() const noexcept {
        return Get();
    }

    [[nodiscard]]
    inline bool IsValid() const noexcept {
        return m_data != nullptr;
    }

    operator bool() const noexcept {
        return IsValid();
    }

    T* operator->() const noexcept {
        return Get();
    }

    T& operator*() const noexcept {
        return *Get();
    }

private:
    T* m_data = nullptr;
};

template <typename T>
class Raw;

template <typename T> requires std::derived_from<T, LLGL::RenderSystemChild>
class Raw<T> {
private:
    Raw(std::shared_ptr<sge::RenderContext> context, T* ptr) :
        m_context(std::move(context)),
        m_ptr(ptr)
    {}

public:
    Raw(const Raw&) = delete;
    Raw& operator=(const Raw&) = delete;

    static Raw Create(std::shared_ptr<sge::RenderContext> context, T* ptr) {
        return Raw(std::move(context), ptr);
    }

    [[nodiscard]]
    Ref<T> AsRef() const noexcept {
        return Ref<T>(m_context, m_ptr);
    }

    [[nodiscard]]
    Unique<T> AsUnique() const noexcept {
        return Unique<T>(m_context, m_ptr);
    }

    [[nodiscard]]
    operator T*() const noexcept {
        return m_ptr;
    }

    [[nodiscard]]
    operator Ref<T>() const noexcept {
        return AsRef();
    }

    [[nodiscard]]
    operator Unique<T>() const noexcept {
        return AsUnique();
    }

    operator bool() const noexcept {
        return m_ptr != nullptr;
    }

private:
    std::shared_ptr<sge::RenderContext> m_context = nullptr;
    T* m_ptr = nullptr;
};

template <typename T> requires (!std::derived_from<T, LLGL::RenderSystemChild>)
class Raw<T> {
private:
    Raw(T* ptr) : m_ptr(ptr) {}

public:
    Raw(const Raw&) = delete;
    Raw& operator=(const Raw&) = delete;

    static Raw Create(T* ptr) {
        return Raw(ptr);
    }

    [[nodiscard]]
    Ref<T> AsRef() const noexcept {
        return Ref<T>(m_ptr);
    }

    [[nodiscard]]
    Unique<T> AsUnique() const noexcept {
        return Unique<T>(m_ptr);
    }

    [[nodiscard]]
    operator T*() const noexcept {
        return m_ptr;
    }

    [[nodiscard]]
    operator Ref<T>() const noexcept {
        return AsRef();
    }

    [[nodiscard]]
    operator Unique<T>() const noexcept {
        return AsUnique();
    }

    operator bool() const noexcept {
        return m_ptr != nullptr;
    }

private:
    T* m_ptr = nullptr;
};

} // namespace sge




#endif // SGE_RENDERER_RESOURCE_HPP_