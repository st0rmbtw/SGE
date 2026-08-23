#ifndef SGE_RENDERER_RESOURCE_HPP_
#define SGE_RENDERER_RESOURCE_HPP_

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>

#include <LLGL/RenderSystemChild.h>
#include <type_traits>

namespace sge {

class RenderContext;

class RefCounted {
public:
    inline void IncRefCount() const {
        ++m_ref_count;
    };

    inline void DecRefCount() const {
        if (m_ref_count > 0) {
            --m_ref_count;
        }
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
    LLGLResourceRC(LLGL::RenderSystemChild* data) : m_data(data) {}

    explicit LLGLResourceRC(std::shared_ptr<RenderContext> context, LLGL::RenderSystemChild* data) :
        m_render_context(std::move(context)),
        m_data(data)
    {
    }
    ~LLGLResourceRC();

    inline LLGL::RenderSystemChild* Get() noexcept {
        return m_data;
    }

private:
    std::shared_ptr<RenderContext> m_render_context = nullptr;
    LLGL::RenderSystemChild* m_data = nullptr;
};

class LLGLResource {
public:
    LLGLResource() = default;
    LLGLResource(std::nullptr_t) {};

    explicit LLGLResource(std::shared_ptr<RenderContext> context, LLGL::RenderSystemChild* data) :
        m_render_context(std::move(context)),
        m_data(data)
    {}

    ~LLGLResource() {
        Destroy();
    }

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

    void Destroy();

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

namespace internal {
    template <typename T>
    inline bool IsDataValid(T* data) {
        return data != nullptr;
    }

    template <>
    inline bool IsDataValid(LLGLResourceRC* data) {
        return data != nullptr && data->Get() != nullptr;
    }

    template <typename T>
    inline bool IsDataValid(const LLGLResource& data) {
        return data.Get() != nullptr;
    }

    template <typename T, typename TReturn>
    inline TReturn* DataGet(T* data) {
        return data;
    }

    template <typename TData, typename TReturn>
    inline TReturn* DataGet(LLGLResourceRC* data) {
        if (data)
            return static_cast<TReturn*>(data->Get());
        return nullptr;
    }

    template <typename TData, typename TReturn>
    inline TReturn* DataGet(const LLGLResource& data) {
        return static_cast<TReturn*>(data.Get());
    }
} // namespace internal

template <typename TData, typename TReturn> requires std::derived_from<TData, RefCounted>
class RefImpl {
public:
    constexpr RefImpl() = default;
    constexpr RefImpl(std::nullptr_t) {}

    explicit RefImpl(TData* data) : 
        m_data(data)
    {
        IncrementRef();
    }

    template <typename... TArgs>
    static RefImpl Create(TArgs&&... args) {
        return RefImpl(new TData(std::forward<TArgs>(args)...));
    }

    ~RefImpl() {
        DecrementRef();
    }

    RefImpl(RefImpl&& other) noexcept {
        operator=(std::move(other));
    }

    RefImpl(const RefImpl& other) {
        operator=(other);
    }

    RefImpl& operator=(RefImpl&& other) noexcept {
        DecrementRef();
        m_data = std::exchange(other.m_data, nullptr);
        return *this;
    }

    RefImpl& operator=(const RefImpl& other) {
        if (this == &other)
            return *this;

        other.IncrementRef();
        DecrementRef();
        m_data = other.m_data;
        return *this;
    }

    TReturn* Get() const noexcept {
        return internal::DataGet<TData, TReturn>(m_data);
    }

    operator TReturn*() const noexcept {
        return Get();
    }

    [[nodiscard]]
    inline bool IsValid() const noexcept {
        return internal::IsDataValid(m_data);
    }

    operator bool() const noexcept {
        return IsValid();
    }

    auto operator->() const noexcept {
        return Get();
    }

    TReturn& operator*() const noexcept {
        return *Get();
    }

    bool operator==(const RefImpl& other) const noexcept {
        return Get() == other.Get();
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
    TData* m_data = nullptr;
};

template <bool a, typename... Args>
struct ref_impl_t;

template<typename T>
struct ref_impl_t<true, T> {
    using type = RefImpl<LLGLResourceRC, T>;
};

template<typename T>
struct ref_impl_t<false, T> {
    using type = RefImpl<T, T>;
};

template <typename T>
using Ref = ref_impl_t<std::is_base_of_v<LLGL::RenderSystemChild, std::remove_cvref_t<T>>, T>::type;

template <typename TData, typename TReturn>
class UniqueImpl {
public:
    UniqueImpl() = default;
    UniqueImpl(std::nullptr_t) {}
    UniqueImpl(TData data) :
        m_data(std::move(data))
    {
    }

    template <typename... TArgs>
    static UniqueImpl Create(TArgs&&... args) requires (!std::is_pointer_v<TData>) {
        return UniqueImpl(TData(std::forward<TArgs>(args)...));
    }

    UniqueImpl(const UniqueImpl&) = delete;
    UniqueImpl& operator=(const UniqueImpl&) = delete;

    UniqueImpl(UniqueImpl&& other) noexcept {
        operator=(std::move(other));
    }
    UniqueImpl& operator=(UniqueImpl&& other) noexcept {
        m_data = std::exchange(other.m_data, nullptr);
        return *this;
    }

    UniqueImpl& operator=(std::nullptr_t) noexcept {
        m_data.Destroy();
        return *this;
    }

    TReturn* Get() const noexcept {
        return internal::DataGet<TData, TReturn>(m_data);
    }

    operator TReturn*() const noexcept {
        return Get();
    }

    [[nodiscard]]
    inline bool IsValid() const noexcept {
        return internal::IsDataValid<TData>(m_data);
    }

    operator bool() const noexcept {
        return IsValid();
    }

    TReturn* operator->() const noexcept {
        return Get();
    }

    TReturn& operator*() const noexcept {
        return *Get();
    }

    ~UniqueImpl() requires std::is_pointer_v<TData> {
        delete m_data;
    }

    ~UniqueImpl() = default;

private:
    TData m_data;
};

template <bool a, typename... Args>
struct unique_impl_t;

template<typename T>
struct unique_impl_t<true, T> {
    using type = UniqueImpl<LLGLResource, T>;
};

template<typename T>
struct unique_impl_t<false, T> {
    using type = UniqueImpl<T, T>;
};

template <typename T>
using Unique = unique_impl_t<std::is_base_of_v<LLGL::RenderSystemChild, std::remove_cvref_t<T>>, T>::type;

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
        return Ref<T>::Create(m_context, m_ptr);
    }

    [[nodiscard]]
    Unique<T> AsUnique() const noexcept {
        return Unique<T>::Create(m_context, m_ptr);
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