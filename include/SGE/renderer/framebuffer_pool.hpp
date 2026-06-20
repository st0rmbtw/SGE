#ifndef SGE_RENDERER_FRAMEBUFFER_POOL_HPP_
#define SGE_RENDERER_FRAMEBUFFER_POOL_HPP_

#include <cstdint>

#include <LLGL/Format.h>

#include <SGE/assert.hpp>
#include <SGE/types/framebuffer.hpp>
#include <SGE/utils/hash.hpp>

namespace sge {
    struct TemporaryFramebufferKey {
        uint32_t width;
        uint32_t height;
        long bindFlags;
        LLGL::Format format;
    };
    
    inline constexpr bool operator==(const sge::TemporaryFramebufferKey& a, const sge::TemporaryFramebufferKey& b) {
        return a.width == b.width && a.height == b.height && a.bindFlags == b.bindFlags && a.format == b.format;
    }
} // namespace sge


template <>
struct std::hash<sge::TemporaryFramebufferKey> {
    size_t operator()(const sge::TemporaryFramebufferKey& key) const noexcept {
        size_t hash = 255323;
        hash_combine(hash, key.width);
        hash_combine(hash, key.height);
        hash_combine(hash, key.width);
        hash_combine(hash, static_cast<int>(key.format));
        return hash;
    }
};

namespace sge {

struct PoolEntry {
    std::shared_ptr<sge::Framebuffer> framebuffer;
    TemporaryFramebufferKey key;
    uint32_t last_used_frame = 0;
    bool occupied = false;
};

class TemporaryFramebufferPool;

class TemporaryFramebuffer {
public:
    TemporaryFramebuffer() = default;

    TemporaryFramebuffer(TemporaryFramebufferPool* pool, sge::PoolEntry* entry) :
        m_pool(pool),
        m_entry(entry)
    {}

    TemporaryFramebuffer(TemporaryFramebuffer&& other) noexcept
        : m_pool(other.m_pool), m_entry(other.m_entry)
    {
        other.m_pool = nullptr;
        other.m_entry = nullptr;
    }

    TemporaryFramebuffer& operator=(TemporaryFramebuffer&& other) noexcept {
        if (this == &other)
            return *this;

        Release();
        m_entry = other.m_entry;
        m_pool = other.m_pool;
        other.m_entry = nullptr;
        other.m_pool = nullptr;

        return *this;
    }

    TemporaryFramebuffer(const TemporaryFramebuffer&) = delete;
    TemporaryFramebuffer& operator=(const TemporaryFramebuffer&) = delete;

    [[nodiscard]]
    inline uint8_t ColorAttachmentCount() const noexcept {
        return m_entry->framebuffer->ColorAttachmentCount();
    }

    [[nodiscard]]
    inline bool HasDepthAttachment() const noexcept {
        return m_entry->framebuffer->HasDepthAttachment();
    }

    [[nodiscard]]
    inline bool HasStencilAttachment() const noexcept {
        return m_entry->framebuffer->HasStencilAttachment();
    }

    [[nodiscard]]
    inline LLGL::Extent2D GetResolution() const noexcept {
        return m_entry->framebuffer->GetResolution();
    }

    [[nodiscard]]
    inline const sge::Ref<LLGL::Texture>& GetTexture(uint8_t i) const noexcept {
        return m_entry->framebuffer->GetTexture(i);
    }

    [[nodiscard]]
    inline const sge::Ref<LLGL::RenderTarget>& GetRenderTarget() const noexcept {
        return m_entry->framebuffer->GetRenderTarget();
    }

    [[nodiscard]]
    inline bool IsValid() const noexcept {
        return m_entry != nullptr;
    }

    void Release();

    ~TemporaryFramebuffer() {
        Release();
    }

private:
    sge::TemporaryFramebufferPool* m_pool = nullptr;
    sge::PoolEntry* m_entry = nullptr;
};

class TemporaryFramebufferPool {
public:
    TemporaryFramebuffer Get(const TemporaryFramebufferKey& key) {
        SGE_ASSERT(key.width != 0 && key.height != 0);
        
        for (auto& entry : m_entries) {
            if (!entry->occupied && entry->key == key) {
                entry->occupied = true;
                entry->last_used_frame = m_current_frame;
                return TemporaryFramebuffer(this, entry.get());
            }
        }

        return TemporaryFramebuffer();
    }

    TemporaryFramebuffer Add(const TemporaryFramebufferKey& key, sge::Framebuffer framebuffer) {
        auto entry = std::make_unique<PoolEntry>();
        entry->framebuffer = std::make_shared<sge::Framebuffer>(std::move(framebuffer));
        entry->key = key;
        entry->last_used_frame = m_current_frame;
        entry->occupied = false;

        PoolEntry* raw = entry.get();
        m_entries.push_back(std::move(entry));
        return TemporaryFramebuffer(this, raw);
    }

    void Tick(uint32_t max_unused_frames = 60) {
        ++m_current_frame;
        std::erase_if(m_entries, [&](const std::unique_ptr<PoolEntry>& e) {
            return !e->occupied && (m_current_frame - e->last_used_frame) > max_unused_frames;
        });
    }
    
private:
    std::vector<std::unique_ptr<PoolEntry>> m_entries;
    uint64_t m_current_frame = 0;
};

} // namespace sge

#endif