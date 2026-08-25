#ifndef _SGE_RENDERER_BATCH_HPP_
#define _SGE_RENDERER_BATCH_HPP_

#include <optional>
#include <vector>

#include <LLGL/RenderSystem.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <SGE/defines.hpp>
#include <SGE/math/rect.hpp>
#include <SGE/renderer/buffer_pool.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/renderer/resource.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/types/color.hpp>
#include <SGE/types/font.hpp>
#include <SGE/types/nine_patch.hpp>
#include <SGE/types/order.hpp>
#include <SGE/types/rich_text.hpp>
#include <SGE/types/shape.hpp>
#include <SGE/types/sprite.hpp>
#include <SGE/types/texture.hpp>
#include <SGE/utils/bitflags.hpp>
#include <SGE/utils/containers/heaparray.hpp>

namespace sge {

class Renderer2D;
class Renderer;

namespace internal {

struct BatchState {
    sge::IRect scissor;
    size_t resources_offset;
    size_t resources_count;
    uint32_t order;
    sge::BlendMode blend_mode;
};

inline static glm::vec4 get_uv_offset_scale(bool flip_x, bool flip_y) {
    glm::vec4 uv_offset_scale = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

    if (flip_x) {
        uv_offset_scale.x += uv_offset_scale.z;
        uv_offset_scale.z *= -1.0f;
    }

    if (flip_y) {
        uv_offset_scale.y += uv_offset_scale.w;
        uv_offset_scale.w *= -1.0f;
    }

    return uv_offset_scale;
}

} // namespace internal

struct DrawCommand {
    size_t instance_index;
    internal::BatchState state;
};

enum class BatchFlags : uint8_t {
    UI = 0
};

struct SpriteBatchDesc {
    sge::Ref<LLGL::Shader> customPixelShader = nullptr;
    bool enableScissor = false;
    bool enableDepth = false;
};

struct NinePatchBatchDesc {
    sge::Ref<LLGL::Shader> customPixelShader = nullptr;
    bool enableScissor = false;
    bool enableDepth = false;
};

struct LineBatchDesc {
    bool enableScissor = false;
    bool enableDepth = false;
};

struct TextSdfBatchDesc {
    sge::Ref<LLGL::Shader> customPixelShader = nullptr;
    bool enableScissor = false;
    bool enableDepth = false;
};

struct TextVectorBatchDesc {
    bool enableScissor = false;
    bool enableDepth = false;
};

struct ShapeBatchDesc {
    bool enableScissor = false;
    bool enableDepth = false;
};

class IBatch : public RefCounted {
protected:
    using FlagsType = sge::BitFlags<BatchFlags>;
public:
    [[nodiscard]]
    virtual const sge::Ref<sge::Mesh>& GetMesh() const noexcept = 0;

    [[nodiscard]]
    virtual const sge::Ref<sge::Material>& GetMaterial() const noexcept = 0;

    virtual void* GetInstanceData() noexcept = 0;

    [[nodiscard]]
    virtual size_t GetInstanceCount() const noexcept = 0;

    [[nodiscard]]
    virtual std::vector<DrawCommand>& GetDrawCommands() noexcept = 0;

    [[nodiscard]]
    virtual LLGL::Resource* const* GetDynamicBindings() const noexcept = 0;

    virtual ~IBatch() = default;

protected:
    virtual void Clear() = 0;

public:
    void SetGlobalFlags(FlagsType flags) noexcept {
        m_flags = flags;
    }

    inline void BeginOrderMode(int order, bool advance) noexcept {
        m_order_mode = true;
        m_global_order.value = order < 0 ? m_order : order;
        m_global_order.advance = advance;
    }

    inline void BeginOrderMode(int order = -1) noexcept {
        BeginOrderMode(order, true);
    }

    inline void BeginOrderMode(bool advance) noexcept {
        BeginOrderMode(-1, advance);
    }

    inline void EndOrderMode() noexcept {
        m_order_mode = false;
        m_global_order.value = 0;
        m_global_order.advance = false;
    }

    inline void BeginScissorMode(sge::IRect scissor) {
        m_scissor_stack.push_back(scissor);
    }

    inline void EndScissorMode() {
        m_scissor_stack.pop_back();
    }

    inline void Reset() {
        Clear();
        m_order = 0;
        m_order_mode = false;
    }

    inline FlagsType GetFlags() const noexcept {
        return m_flags;
    }

    inline std::optional<sge::IRect> GetCurrentScissor() const noexcept {
        return m_scissor_stack.empty() ? std::nullopt : std::optional(m_scissor_stack.back());
    }

protected:
    inline uint32_t GetOrder(sge::Order customOrder) {
        const uint32_t order = m_order_mode
            ? m_global_order.value + std::max(customOrder.value, 0)
            : (customOrder.value >= 0 ? customOrder.value : m_order);

        customOrder.advance |= m_global_order.advance;

        if (customOrder.advance)
            m_order = std::max(m_order, order + 1);

        return order;
    }

private:
    std::vector<sge::IRect> m_scissor_stack;
    Order m_global_order;
    uint32_t m_order = 0;
    bool m_order_mode = false;
    FlagsType m_flags;
};

class SpriteBatch : public IBatch {
public:
    explicit SpriteBatch(sge::Renderer& renderer, SpriteBatchDesc desc = {});

    uint32_t Draw(const Sprite& sprite, sge::Order customOrder = {}, std::optional<FlagsType> overrideFlags = std::nullopt) {
        const glm::vec4 uv_offset_scale = internal::get_uv_offset_scale(sprite.flip_x(), sprite.flip_y());
        return AddSpriteDrawCommand(sprite, uv_offset_scale, sprite.texture(), overrideFlags, customOrder);
    }

    uint32_t DrawAtlasSprite(const sge::TextureAtlasSprite& sprite, sge::Order customOrder = {}, std::optional<FlagsType> overrideFlags = std::nullopt);

    const sge::Ref<sge::Mesh>& GetMesh() const noexcept override {
        return m_mesh;
    }

    const sge::Ref<sge::Material>& GetMaterial() const noexcept override {
        return m_material;
    }

    void* GetInstanceData() noexcept override {
        return m_instances.data();
    }

    size_t GetInstanceCount() const noexcept override {
        return m_instances.size();
    }

    std::vector<DrawCommand>& GetDrawCommands() noexcept override {
        return m_commands;
    }

    LLGL::Resource* const* GetDynamicBindings() const noexcept override {
        return m_dynamic_bindings.data();
    }

    ~SpriteBatch() override = default;

private:
    void Clear() override {
        m_commands.clear();
        m_instances.clear();
    }

private:
    uint32_t AddSpriteDrawCommand(const sge::BaseSprite& sprite, const glm::vec4& uv_offset_scale, const sge::Texture& texture, std::optional<FlagsType> override_flags, sge::Order custom_order);

private:
    std::vector<DrawCommand> m_commands;
    std::vector<SpriteInstance> m_instances;
    std::vector<LLGL::Resource*> m_dynamic_bindings;
    sge::Ref<sge::Mesh> m_mesh;
    sge::Ref<sge::Material> m_material;
};

class NinePatchBatch : public IBatch {
public:
    explicit NinePatchBatch(sge::Renderer& renderer, NinePatchBatchDesc desc = {});

    uint32_t Draw(const NinePatch& sprite, sge::Order order = {}, std::optional<FlagsType> overrideFlags = std::nullopt);

    const sge::Ref<sge::Mesh>& GetMesh() const noexcept override {
        return m_mesh;
    }

    const sge::Ref<sge::Material>& GetMaterial() const noexcept override {
        return m_material;
    }

    void* GetInstanceData() noexcept override {
        return m_instances.data();
    }

    size_t GetInstanceCount() const noexcept override {
        return m_instances.size();
    }

    std::vector<DrawCommand>& GetDrawCommands() noexcept override {
        return m_commands;
    }

    LLGL::Resource* const* GetDynamicBindings() const noexcept override {
        return m_dynamic_bindings.data();
    }

    ~NinePatchBatch() override = default;

private:
    void Clear() override {
        m_commands.clear();
        m_instances.clear();
    }

private:
    std::vector<DrawCommand> m_commands;
    std::vector<NinePatchInstance> m_instances;
    std::vector<LLGL::Resource*> m_dynamic_bindings;
    sge::Ref<sge::Mesh> m_mesh;
    sge::Ref<sge::Material> m_material;
};

class LineBatch : public IBatch {
public:
    explicit LineBatch(sge::Renderer& renderer, LineBatchDesc desc = {});

    uint32_t Draw(glm::vec2 start, glm::vec2 end, float thickness, const sge::LinearRgba& color, BorderRadius borderRadius = BorderRadius(), sge::Order customOrder = {}, std::optional<FlagsType> overrideFlags = std::nullopt);

    const sge::Ref<sge::Mesh>& GetMesh() const noexcept override {
        return m_mesh;
    }

    const sge::Ref<sge::Material>& GetMaterial() const noexcept override {
        return m_material;
    }

    void* GetInstanceData() noexcept override {
        return m_instances.data();
    }

    size_t GetInstanceCount() const noexcept override {
        return m_instances.size();
    }

    std::vector<DrawCommand>& GetDrawCommands() noexcept override {
        return m_commands;
    }

    LLGL::Resource* const* GetDynamicBindings() const noexcept override {
        return nullptr;
    }

    ~LineBatch() override = default;

private:
    void Clear() override {
        m_commands.clear();
        m_instances.clear();
    }

private:
    std::vector<DrawCommand> m_commands;
    std::vector<LineInstance> m_instances;
    sge::Ref<sge::Mesh> m_mesh;
    sge::Ref<sge::Material> m_material;
};

class TextSdfBatch : public IBatch {
public:
    explicit TextSdfBatch(sge::Renderer& renderer, TextSdfBatchDesc desc = {});

    uint32_t Draw(const sge::RichTextSection* sections, size_t size, glm::vec2 position, const sge::Font& font, sge::Order order, std::optional<FlagsType> overrideFlags = std::nullopt);

    template <size_t Size>
    inline uint32_t Draw(const sge::RichText<Size>& text, glm::vec2 position, const sge::Font& font, sge::Order order = {}, std::optional<FlagsType> overrideFlags = std::nullopt) {
        return Draw(text.sections, Size, position, font, order, overrideFlags);
    }

    inline uint32_t Draw(const std::string& text, float size, sge::LinearRgba color, glm::vec2 position, const sge::Font& font, sge::Order order = {}, std::optional<FlagsType> overrideFlags = std::nullopt) {
        sge::RichTextSection section = { .text=text, .color=color, .size=size };
        return Draw(&section, 1, position, font, order, overrideFlags);
    }

    const sge::Ref<sge::Mesh>& GetMesh() const noexcept override {
        return m_mesh;
    }

    const sge::Ref<sge::Material>& GetMaterial() const noexcept override {
        return m_material;
    }

    void* GetInstanceData() noexcept override {
        return m_instances.data();
    }

    size_t GetInstanceCount() const noexcept override {
        return m_instances.size();
    }

    std::vector<DrawCommand>& GetDrawCommands() noexcept override {
        return m_commands;
    }

    LLGL::Resource* const* GetDynamicBindings() const noexcept override {
        return m_dynamic_bindings.data();
    }

    ~TextSdfBatch() override = default;

private:
    void Clear() override {
        m_commands.clear();
        m_instances.clear();
    }

private:
    std::vector<DrawCommand> m_commands;
    std::vector<GlyphInstanceSDF> m_instances;
    std::vector<LLGL::Resource*> m_dynamic_bindings;
    sge::Ref<sge::Mesh> m_mesh;
    sge::Ref<sge::Material> m_material;
};

class TextVectorBatch : public IBatch {
public:
    explicit TextVectorBatch(sge::Renderer& renderer, TextVectorBatchDesc desc = {});

    uint32_t Draw(const sge::RichTextSection* sections, size_t size, glm::vec2 position, const sge::FontVector& font, sge::Order order, std::optional<FlagsType> overrideFlags);

    template <size_t Size>
    inline uint32_t Draw(const sge::RichText<Size>& text, glm::vec2 position, const sge::FontVector& font, sge::Order order = {}, std::optional<FlagsType> overrideFlags = std::nullopt) {
        return Draw(text.sections, Size, position, font, order, overrideFlags);
    }

    inline uint32_t Draw(const std::string& text, float size, sge::LinearRgba color, glm::vec2 position, const sge::FontVector& font, sge::Order order = {}, std::optional<FlagsType> overrideFlags = std::nullopt) {
        sge::RichTextSection section = { .text=text, .color=color, .size=size };
        return Draw(&section, 1, position, font, order, overrideFlags);
    }

    const sge::Ref<sge::Mesh>& GetMesh() const noexcept override {
        return m_mesh;
    }

    const sge::Ref<sge::Material>& GetMaterial() const noexcept override {
        return m_material;
    }

    void* GetInstanceData() noexcept override {
        return m_instances.data();
    }

    size_t GetInstanceCount() const noexcept override {
        return m_instances.size();
    }

    std::vector<DrawCommand>& GetDrawCommands() noexcept override {
        return m_commands;
    }

    LLGL::Resource* const* GetDynamicBindings() const noexcept override {
        return m_dynamic_bindings.data();
    }

    ~TextVectorBatch() override = default;

private:
    void Clear() override {
        m_commands.clear();
        m_instances.clear();
    }

private:
    std::vector<DrawCommand> m_commands;
    std::vector<GlyphInstanceVector> m_instances;
    std::vector<LLGL::Resource*> m_dynamic_bindings;
    sge::Ref<sge::Mesh> m_mesh;
    sge::Ref<sge::Material> m_material;
};

class ShapeBatch : public IBatch {
public:
    explicit ShapeBatch(sge::Renderer& renderer, ShapeBatchDesc desc = {});

    uint32_t Draw(sge::Shape::Type shape, glm::vec2 position, glm::vec2 size, const sge::LinearRgba& color, const sge::LinearRgba& borderColor, float borderThickness, BorderRadius borderRadius = BorderRadius(), sge::Anchor anchor = sge::Anchor::Center, sge::Order customOrder = {}, std::optional<FlagsType> overrideFlags = std::nullopt);

    inline uint32_t DrawCircle(glm::vec2 position, sge::Order customOrder, std::optional<FlagsType> overrideFlags, const ShapeCircle& circle) {
        return Draw(sge::Shape::Circle, position, glm::vec2(circle.radius * 2.0f), circle.color, circle.border_color, circle.border_thickness, BorderRadius::Absolute(0.0), circle.anchor, customOrder, overrideFlags);
    }

    inline uint32_t DrawCircle(glm::vec2 position, const ShapeCircle& circle) {
        return DrawCircle(position, sge::Order(), std::nullopt, circle);
    }

    inline uint32_t DrawRect(glm::vec2 position, sge::Order customOrder, std::optional<FlagsType> overrideFlags, const ShapeRect& rect) {
        return Draw(sge::Shape::Rect, position, rect.size, rect.color, rect.border_color, rect.border_thickness, rect.border_radius, rect.anchor, customOrder, overrideFlags);
    }

    inline uint32_t DrawRect(glm::vec2 position, const ShapeRect& rect) {
        return DrawRect(position, sge::Order(), std::nullopt, rect);
    }

    inline uint32_t DrawArc(glm::vec2 position, sge::Order customOrder, std::optional<FlagsType> overrideFlags, const ShapeArc& arc) {
        return Draw(sge::Shape::Arc, position, glm::vec2(arc.outer_radius * 2.0f), arc.color, sge::LinearRgba(0.0f), arc.inner_radius, BorderRadius::Absolute(arc.start_angle, arc.end_angle, 0.0f, 0.0f), arc.anchor, customOrder, overrideFlags);
    }

    inline uint32_t DrawArc(glm::vec2 position, const ShapeArc& arc) {
        return DrawArc(position, sge::Order(), std::nullopt, arc);
    }

    const sge::Ref<sge::Mesh>& GetMesh() const noexcept override {
        return m_mesh;
    }

    const sge::Ref<sge::Material>& GetMaterial() const noexcept override {
        return m_material;
    }

    void* GetInstanceData() noexcept override {
        return m_instances.data();
    }

    size_t GetInstanceCount() const noexcept override {
        return m_instances.size();
    }

    std::vector<DrawCommand>& GetDrawCommands() noexcept override {
        return m_commands;
    }

    LLGL::Resource* const* GetDynamicBindings() const noexcept override {
        return nullptr;
    }

    ~ShapeBatch() override = default;

private:
    void Clear() override {
        m_commands.clear();
        m_instances.clear();
    }

private:
    std::vector<DrawCommand> m_commands;
    std::vector<ShapeInstance> m_instances;
    sge::Ref<sge::Mesh> m_mesh;
    sge::Ref<sge::Material> m_material;
};

/**
 * @brief Helper for beginning order mode on multiple batches at once.
 */
template <typename... Instances>
void BeginOrderModes(int order, Instances&&... instances) {
    auto method_ptr = static_cast<void (IBatch::*)(int) noexcept>(&IBatch::BeginOrderMode);
    (..., std::invoke(method_ptr, std::forward<Instances>(instances), order));
}

/**
 * @brief Helper for beginning order mode on multiple batches at once.
 */
template <typename... Instances>
void BeginOrderModes(int order, bool advance, Instances&&... instances) {
    auto method_ptr = static_cast<void (IBatch::*)(int, bool) noexcept>(&IBatch::BeginOrderMode);
    (..., std::invoke(method_ptr, std::forward<Instances>(instances), order, advance));
}

/**
 * @brief Helper for beginning order mode on multiple batches at once.
 */
template <typename... Instances>
void BeginOrderModes(Instances&&... instances) {
    auto method_ptr = static_cast<void (IBatch::*)(int) noexcept>(&IBatch::BeginOrderMode);
    (..., std::invoke(method_ptr, std::forward<Instances>(instances), -1));
}

// End of recursion
inline void EndOrderModes() {}

/**
 * @brief Helper for ending order mode on multiple batches at once.
 */
template <typename FirstInstance, typename... Instances>
void EndOrderModes(FirstInstance&& first, Instances&&... other) {
    EndOrderModes(std::forward<Instances>(other)...);
    std::invoke(&IBatch::EndOrderMode, std::forward<FirstInstance>(first));
}

/**
 * @brief Helper for beginning scissor mode on multiple batches at once.
 */
template <typename... Instances>
void BeginScissorModes(sge::IRect scissor, Instances&&... instances) {
    (..., std::invoke(&IBatch::BeginScissorMode, std::forward<Instances>(instances), scissor));
}

/**
 * @brief Helper for ending scissor mode on multiple batches at once.
 */
template <typename... Instances>
void EndScissorModes(Instances&&... instances) {
    (..., std::invoke(&IBatch::EndScissorMode, std::forward<Instances>(instances)));
}

/**
 * @brief Helper for resetting multiple batches at once.
 *
 * @tparam Instances Types of batch instances, e.g. a pointer, reference, shared_ptr, unique_ptr, etc.
 * @param instances Batch instances
 */
template <typename... Instances>
void ResetBatches(Instances&&... instances) {
    (..., std::invoke(&IBatch::Reset, std::forward<Instances>(instances)));
}

} // namespace sge

#endif