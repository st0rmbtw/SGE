#ifndef SGE_RENDERER_RESOURCE_HPP_
#define SGE_RENDERER_RESOURCE_HPP_

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>

#include <LLGL/RenderSystemChild.h>

#include <SGE/assert.hpp>

namespace sge {

namespace internal {
    template <typename T, typename Base>
    concept is_derived_array =
        std::is_array_v<T> &&
        std::derived_from<std::remove_pointer_t<std::remove_all_extents_t<T>>, Base>;
}

class RenderContext;

class RefCounted {
    template <typename TData> requires std::derived_from<TData, RefCounted>
    friend class RefBaseImpl;
public:
    [[nodiscard]]
    inline uint32_t GetRefCount() const noexcept {
        return m_ref_count;
    }

private:
    inline void IncRefCount() const {
        ++m_ref_count;
    };

    inline void DecRefCount() const {
        if (m_ref_count > 0) {
            --m_ref_count;
        }
    };

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
        Destroy();
        m_render_context = std::exchange(other.m_render_context, nullptr);
        m_data = std::exchange(other.m_data, nullptr);
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

template <typename TData> requires std::derived_from<TData, RefCounted>
class RefBaseImpl {
public:
    constexpr RefBaseImpl() = default;
    constexpr RefBaseImpl(std::nullptr_t) {}

    explicit RefBaseImpl(TData* data) :
        m_data(data)
    {
        IncrementRef();
    }

    ~RefBaseImpl() {
        DecrementRef();
    }

    RefBaseImpl(RefBaseImpl&& other) noexcept {
        operator=(std::move(other));
    }

    RefBaseImpl(const RefBaseImpl& other) {
        operator=(other);
    }

    RefBaseImpl& operator=(RefBaseImpl&& other) noexcept {
        DecrementRef();
        m_data = std::exchange(other.m_data, nullptr);
        return *this;
    }

    RefBaseImpl& operator=(const RefBaseImpl& other) {
        if (this == &other)
            return *this;

        other.IncrementRef();
        DecrementRef();
        m_data = other.m_data;
        return *this;
    }

protected:
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
protected:
    TData* m_data = nullptr;
};

template <typename T>
class Ref;

template <typename T> requires std::derived_from<T, LLGL::RenderSystemChild>
class Ref<T> : public RefBaseImpl<LLGLResourceRC> {
public:
    using RefBaseImpl<LLGLResourceRC>::RefBaseImpl;

    template <typename... TArgs>
    static Ref<T> Create(TArgs&&... args) {
        return Ref(new LLGLResourceRC(std::forward<TArgs>(args)...));
    }

    T* Get() const noexcept {
        if (m_data)
            return static_cast<T*>(m_data->Get());
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

    auto operator->() const noexcept {
        return Get();
    }

    T& operator*() const noexcept {
        return *Get();
    }

    bool operator==(const Ref& other) const noexcept {
        return Get() == other.Get();
    }
};

template <typename T> requires (!std::derived_from<T, LLGL::RenderSystemChild>)
class Ref<T> : public RefBaseImpl<T> {
public:
    using RefBaseImpl<T>::RefBaseImpl;

    template <typename... TArgs>
    static Ref<T> Create(TArgs&&... args) {
        return Ref(new T(std::forward<TArgs>(args)...));
    }

    T* Get() const noexcept {
        return this->m_data;
    }

    operator T*() const noexcept {
        return Get();
    }

    [[nodiscard]]
    inline bool IsValid() const noexcept {
        return this->m_data != nullptr;
    }

    operator bool() const noexcept {
        return IsValid();
    }

    auto operator->() const noexcept {
        return Get();
    }

    T& operator*() const noexcept {
        return *Get();
    }

    bool operator==(const Ref& other) const noexcept {
        return Get() == other.Get();
    }
};

template <typename TData>
class UniqueBaseImpl {
public:
    UniqueBaseImpl() = default;
    UniqueBaseImpl(std::nullptr_t) {}
    UniqueBaseImpl(TData data) :
        m_data(std::move(data))
    {
    }

    UniqueBaseImpl(const UniqueBaseImpl&) = delete;
    UniqueBaseImpl& operator=(const UniqueBaseImpl&) = delete;

    UniqueBaseImpl(UniqueBaseImpl&& other) noexcept {
        operator=(std::move(other));
    }
    UniqueBaseImpl& operator=(UniqueBaseImpl&& other) noexcept {
        m_data = std::exchange(other.m_data, nullptr);
        return *this;
    }

    UniqueBaseImpl& operator=(std::nullptr_t) noexcept {
        m_data.Destroy();
        return *this;
    }

    ~UniqueBaseImpl() requires std::is_pointer_v<TData> {
        delete m_data;
    }

    ~UniqueBaseImpl() = default;

protected:
    TData m_data;
};

struct ResourceDeleter {
    std::shared_ptr<sge::RenderContext> ctx;

    ResourceDeleter() = default;

    explicit ResourceDeleter(std::shared_ptr<sge::RenderContext> context) :
        ctx{ std::move(context) }
    {}

    void operator()(LLGL::RenderSystemChild* child) const noexcept;
};

template <typename T>
class Unique;

template <typename T> requires std::derived_from<T, LLGL::RenderSystemChild>
class Unique<T> final : std::unique_ptr<T, ResourceDeleter> {
    using base_t = std::unique_ptr<T, ResourceDeleter>;
public:
    Unique() : base_t(nullptr, ResourceDeleter{}) {}
    Unique(std::nullptr_t) : base_t(nullptr, ResourceDeleter{}) {}

    template <typename... Args>
    static Unique Create(std::shared_ptr<sge::RenderContext> context, Args&&... args) {
        return Unique(std::move(context), new T(std::forward<Args>(args)...));
    }

    explicit Unique(std::shared_ptr<sge::RenderContext> context, T* ptr) : base_t(ptr, ResourceDeleter(std::move(context)))
    {}

    base_t::pointer Get() const {
        return base_t::get();
    }

    T& operator*() const {
        SGE_ASSERT_M(base_t::get() != nullptr, "null pointer dereference!");
        return *base_t::get();
    }

    explicit operator bool() const noexcept {
        return base_t::operator bool();
    }

    base_t::pointer Release() noexcept {
        return base_t::release();
    }

    void Swap(base_t& other) noexcept {
        base_t::swap(other);
    }

    base_t::pointer operator->() const noexcept {
        return base_t::get();
    }

    operator T*() const noexcept {
        return Get();
    }
};

template <typename T> requires (!std::derived_from<T, LLGL::RenderSystemChild>)
class Unique<T> final : std::unique_ptr<T> {
    using base_t = std::unique_ptr<T>;
public:
    Unique() : base_t() {}
    Unique(std::nullptr_t) : base_t(nullptr) {}

    template <typename... Args>
    static Unique Create(Args&&... args) {
        return Unique(new T(std::forward<Args>(args)...));
    }

    explicit Unique(T* ptr) : base_t(ptr)
    {}

    base_t::pointer Get() const {
        return base_t::get();
    }

    T& operator*() const {
        SGE_ASSERT_M(base_t::get() != nullptr, "null pointer dereference!");
        return *base_t::get();
    }

    explicit operator bool() const noexcept {
        return base_t::operator bool();
    }

    base_t::pointer Release() noexcept {
        return base_t::release();
    }

    void Swap(base_t& other) noexcept {
        base_t::swap(other);
    }

    base_t::pointer operator->() const noexcept {
        return base_t::get();
    }

    operator T*() const noexcept {
        return Get();
    }
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
        return Ref<T>::Create(m_context, m_ptr);
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
