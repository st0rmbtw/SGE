#ifndef SGE_RENDERER_BATCH_HPP_
#define SGE_RENDERER_BATCH_HPP_

#include <SGE/utils/frame_arena.hpp>
#include <optional>
#include <utility>
#include <vector>

#include <LLGL/RenderSystem.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <SGE/defines.hpp>
#include <SGE/math/rect.hpp>
#include <SGE/renderer/buffer_pool.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/renderer/material.hpp>
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

struct DrawCommandState {
    sge::IRect scissor;
    size_t instance_index;
    uint64_t resources_hash;
    size_t resources_offset;
    size_t resources_count;
    uint32_t order;
    sge::BlendMode blend_mode;
};

inline std::atomic<size_t> g_batch_id_counter = 0;

template <class T>
struct BatchTypeIdentifier {
    inline static const size_t id = g_batch_id_counter++;
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

enum class TextAlignment : uint8_t {
    Top = 0,
    Center = 1,
    Bottom = 2
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

struct DrawState {
    sge::IRect scissor;
    uint32_t order;
    sge::BlendMode blendMode;
    std::optional<sge::BitFlags<BatchFlags>> customFlags;
};

class IBatch : public RefCounted {
protected:
    using FlagsType = sge::BitFlags<BatchFlags>;

    IBatch() = default;

public:
    [[nodiscard]]
    virtual const sge::Ref<sge::Mesh>& GetMesh() const noexcept = 0;

    [[nodiscard]]
    virtual const sge::Ref<sge::Material>& GetMaterial(sge::BlendMode blendMode) const noexcept = 0;

    virtual void* GetInstanceData() noexcept = 0;

    [[nodiscard]]
    virtual size_t GetInstanceCount() const noexcept = 0;

    [[nodiscard]]
    virtual std::vector<internal::DrawCommandState>& GetDrawCommands() noexcept = 0;

    [[nodiscard]]
    virtual LLGL::Resource* const* GetDynamicBindings() const noexcept = 0;

    virtual ~IBatch() = default;

protected:
    virtual void Clear() = 0;

public:
    void SetGlobalFlags(FlagsType flags) noexcept {
        m_flags = flags;
    }

    void Reset() {
        Clear();
    }

    FlagsType GetFlags() const noexcept {
        return m_flags;
    }

private:
    FlagsType m_flags;
};

class SpriteBatch final : public IBatch {
public:
    explicit SpriteBatch(sge::Renderer& renderer, SpriteBatchDesc desc = {});

    void Add(const Sprite& sprite, const DrawState& state) {
        const glm::vec4 uv_offset_scale = internal::get_uv_offset_scale(sprite.flip_x(), sprite.flip_y());
        return AddSpriteDrawCommand(sprite, uv_offset_scale, sprite.texture(), state);
    }
    void Add(const TextureAtlasSprite& sprite, const DrawState& state);

    const sge::Ref<sge::Mesh>& GetMesh() const noexcept override {
        return m_mesh;
    }

    const sge::Ref<sge::Material>& GetMaterial(sge::BlendMode blendMode) const noexcept override {
        return m_materials[static_cast<uint8_t>(blendMode)];
    }

    void* GetInstanceData() noexcept override {
        return m_instances.data();
    }

    size_t GetInstanceCount() const noexcept override {
        return m_instances.size();
    }

    std::vector<internal::DrawCommandState>& GetDrawCommands() noexcept override {
        return m_commands;
    }

    LLGL::Resource* const* GetDynamicBindings() const noexcept override {
        return m_dynamic_bindings.data();
    }

    ~SpriteBatch() override = default;

protected:
    void Clear() override {
        m_commands.clear();
        m_instances.clear();
        m_dynamic_bindings.clear();
    }

private:
    void AddSpriteDrawCommand(const sge::BaseSprite& sprite, const glm::vec4& uv_offset_scale, const sge::Texture& texture, const DrawState& state);

private:
    std::vector<internal::DrawCommandState> m_commands;
    std::vector<SpriteInstance> m_instances;
    std::vector<LLGL::Resource*> m_dynamic_bindings;
    sge::Ref<sge::Mesh> m_mesh;
    sge::Ref<sge::Material> m_materials[4];
};

class NinePatchBatch final : public IBatch {
public:
    explicit NinePatchBatch(sge::Renderer& renderer, NinePatchBatchDesc desc = {});

    void Add(const NinePatch& ninepatch, const DrawState& state);

    const sge::Ref<sge::Mesh>& GetMesh() const noexcept override {
        return m_mesh;
    }

    const sge::Ref<sge::Material>& GetMaterial(sge::BlendMode blendMode) const noexcept override {
        return m_materials[static_cast<uint8_t>(blendMode)];
    }

    void* GetInstanceData() noexcept override {
        return m_instances.data();
    }

    size_t GetInstanceCount() const noexcept override {
        return m_instances.size();
    }

    std::vector<internal::DrawCommandState>& GetDrawCommands() noexcept override {
        return m_commands;
    }

    LLGL::Resource* const* GetDynamicBindings() const noexcept override {
        return m_dynamic_bindings.data();
    }

    ~NinePatchBatch() override = default;

protected:
    void Clear() override {
        m_commands.clear();
        m_instances.clear();
        m_dynamic_bindings.clear();
    }

private:
    std::vector<internal::DrawCommandState> m_commands;
    std::vector<NinePatchInstance> m_instances;
    std::vector<LLGL::Resource*> m_dynamic_bindings;
    sge::Ref<sge::Mesh> m_mesh;
    sge::Ref<sge::Material> m_materials[4];
};

struct LineDrawData {
    glm::vec2 start;
    glm::vec2 end;
    float thickness = 1.0f;
    sge::LinearRgba color = sge::color::WHITE;
    BorderRadius borderRadius = BorderRadius();
};

class LineBatch final : public IBatch {
public:
    explicit LineBatch(sge::Renderer& renderer, LineBatchDesc desc = {});

    void Add(const LineDrawData& data, const DrawState& state);

    const sge::Ref<sge::Mesh>& GetMesh() const noexcept override {
        return m_mesh;
    }

    const sge::Ref<sge::Material>& GetMaterial(sge::BlendMode blendMode) const noexcept override {
        return m_materials[static_cast<uint8_t>(blendMode)];
    }

    void* GetInstanceData() noexcept override {
        return m_instances.data();
    }

    size_t GetInstanceCount() const noexcept override {
        return m_instances.size();
    }

    std::vector<internal::DrawCommandState>& GetDrawCommands() noexcept override {
        return m_commands;
    }

    LLGL::Resource* const* GetDynamicBindings() const noexcept override {
        return nullptr;
    }

    ~LineBatch() override = default;

protected:
    void Clear() override {
        m_commands.clear();
        m_instances.clear();
    }

private:
    std::vector<internal::DrawCommandState> m_commands;
    std::vector<LineInstance> m_instances;
    sge::Ref<sge::Mesh> m_mesh;
    sge::Ref<sge::Material> m_materials[4];
};

struct SdfTextDrawData {
    std::span<const sge::RichTextSection> sections;
    const sge::Font& font;
    glm::vec2 position;
    sge::TextAlignment alignment;
};

class TextSdfBatch final : public IBatch {
public:
    explicit TextSdfBatch(sge::Renderer& renderer, TextSdfBatchDesc desc = {});

    void Add(const SdfTextDrawData& data, const DrawState& state);

    const sge::Ref<sge::Mesh>& GetMesh() const noexcept override {
        return m_mesh;
    }

    const sge::Ref<sge::Material>& GetMaterial(sge::BlendMode blendMode) const noexcept override {
        return m_materials[static_cast<uint8_t>(blendMode)];
    }

    void* GetInstanceData() noexcept override {
        return m_instances.data();
    }

    size_t GetInstanceCount() const noexcept override {
        return m_instances.size();
    }

    std::vector<internal::DrawCommandState>& GetDrawCommands() noexcept override {
        return m_commands;
    }

    LLGL::Resource* const* GetDynamicBindings() const noexcept override {
        return m_dynamic_bindings.data();
    }

    ~TextSdfBatch() override = default;

protected:
    void Clear() override {
        m_commands.clear();
        m_instances.clear();
        m_dynamic_bindings.clear();
    }

private:
    std::vector<internal::DrawCommandState> m_commands;
    std::vector<GlyphInstanceSDF> m_instances;
    std::vector<LLGL::Resource*> m_dynamic_bindings;
    sge::Ref<sge::Mesh> m_mesh;
    sge::Ref<sge::Material> m_materials[4];
};

struct VectorTextDrawData {
    std::span<const sge::RichTextSection> sections;
    const sge::FontVector& font;
    glm::vec2 position;
    sge::TextAlignment alignment;
};

class TextVectorBatch final : public IBatch {
public:
    explicit TextVectorBatch(sge::Renderer& renderer, TextVectorBatchDesc desc = {});

    void Add(const VectorTextDrawData& data, const DrawState& state);

    const sge::Ref<sge::Mesh>& GetMesh() const noexcept override {
        return m_mesh;
    }

    const sge::Ref<sge::Material>& GetMaterial(sge::BlendMode blendMode) const noexcept override {
        return m_materials[static_cast<uint8_t>(blendMode)];
    }

    void* GetInstanceData() noexcept override {
        return m_instances.data();
    }

    size_t GetInstanceCount() const noexcept override {
        return m_instances.size();
    }

    std::vector<internal::DrawCommandState>& GetDrawCommands() noexcept override {
        return m_commands;
    }

    LLGL::Resource* const* GetDynamicBindings() const noexcept override {
        return m_dynamic_bindings.data();
    }

    ~TextVectorBatch() override = default;

protected:
    void Clear() override {
        m_commands.clear();
        m_instances.clear();
        m_dynamic_bindings.clear();
    }

private:
    std::vector<internal::DrawCommandState> m_commands;
    std::vector<GlyphInstanceVector> m_instances;
    std::vector<LLGL::Resource*> m_dynamic_bindings;
    sge::Ref<sge::Mesh> m_mesh;
    sge::Ref<sge::Material> m_materials[4];
};

struct ShapeDrawData {
    glm::vec2 position;
    glm::vec2 size;
    sge::LinearRgba color;
    sge::LinearRgba borderColor;
    float borderThickness;
    BorderRadius borderRadius = BorderRadius();
    sge::Anchor anchor = sge::Anchor::Center;
    sge::Shape::Type type;
};

class ShapeBatch final : public IBatch {
public:
    explicit ShapeBatch(sge::Renderer& renderer, ShapeBatchDesc desc = {});

    void Add(const ShapeDrawData& data, const DrawState& state);

    const sge::Ref<sge::Mesh>& GetMesh() const noexcept override {
        return m_mesh;
    }

    const sge::Ref<sge::Material>& GetMaterial(sge::BlendMode blendMode) const noexcept override {
        return m_materials[static_cast<uint8_t>(blendMode)];
    }

    void* GetInstanceData() noexcept override {
        return m_instances.data();
    }

    size_t GetInstanceCount() const noexcept override {
        return m_instances.size();
    }

    std::vector<internal::DrawCommandState>& GetDrawCommands() noexcept override {
        return m_commands;
    }

    LLGL::Resource* const* GetDynamicBindings() const noexcept override {
        return nullptr;
    }

    ~ShapeBatch() override = default;

protected:
    void Clear() override {
        m_commands.clear();
        m_instances.clear();
    }

private:
    std::vector<internal::DrawCommandState> m_commands;
    std::vector<ShapeInstance> m_instances;
    sge::Ref<sge::Mesh> m_mesh;
    sge::Ref<sge::Material> m_materials[4];
};

template<typename Item>
class DrawCall {
public:
    explicit DrawCall(Item item) : m_item(std::move(item)) {}

    DrawCall& WithCustomOrder(sge::Order order) & {
        m_order = order;
        return *this;
    }

    DrawCall& WithCustomFlags(std::optional<sge::BitFlags<BatchFlags>> flags) & {
        m_flags = flags;
        return *this;
    }

    DrawCall WithCustomOrder(sge::Order order) && {
        m_order = order;
        return std::move(*this);
    }

    DrawCall WithCustomFlags(std::optional<sge::BitFlags<BatchFlags>> flags) && {
        m_flags = flags;
        return std::move(*this);
    }

    [[nodiscard]]
    const Item& GetItem() const noexcept {
        return m_item;
    }

    [[nodiscard]]
    sge::Order GetCustomOrder() const noexcept {
        return m_order;
    };

    [[nodiscard]]
    std::optional<sge::BitFlags<BatchFlags>> GetCustomFlags() const noexcept {
        return m_flags;
    };

private:
    Item m_item;
    sge::Order m_order = sge::Order();
    std::optional<sge::BitFlags<BatchFlags>> m_flags = std::nullopt;
};

namespace Draw {
    inline DrawCall<const Sprite&> Sprite(const Sprite& sprite) {
        return DrawCall<const class Sprite&>(sprite);
    }

    inline DrawCall<const TextureAtlasSprite&> AtlasSprite(const TextureAtlasSprite& sprite) {
        return DrawCall<const TextureAtlasSprite&>(sprite);
    }

    inline DrawCall<ShapeDrawData> Rect(glm::vec2 position, const ShapeRect& rect) {
        return DrawCall(ShapeDrawData {
            .position = position,
            .size = rect.size,
            .color = rect.color,
            .borderColor = rect.borderColor,
            .borderThickness = rect.borderThickness,
            .borderRadius = rect.borderRadius,
            .anchor = rect.anchor,
            .type = Shape::Rect
        });
    }

    inline DrawCall<ShapeDrawData> Circle(glm::vec2 position, const ShapeCircle& circle) {
        return DrawCall(ShapeDrawData {
            .position = position,
            .size = glm::vec2(circle.radius * 2.0f),
            .color = circle.color,
            .borderColor = circle.borderColor,
            .borderThickness = circle.borderThickness,
            .borderRadius = BorderRadius::Absolute(0.0),
            .anchor = circle.anchor,
            .type = Shape::Circle
        });
    }

    inline DrawCall<ShapeDrawData> Arc(glm::vec2 position, const ShapeArc& arc) {
        return DrawCall(ShapeDrawData {
            .position = position,
            .size = glm::vec2(arc.outerRadius * 2.0f),
            .color = arc.color,
            .borderColor = sge::color::TRANSPARENT,
            .borderThickness = arc.innerRadius,
            .borderRadius = BorderRadius::Absolute(arc.startAngle, arc.endAngle, 0.0f, 0.0f),
            .anchor = arc.anchor,
            .type = Shape::Arc
        });
    }

    inline DrawCall<LineDrawData> Line(glm::vec2 start, glm::vec2 end, float thickness, const sge::LinearRgba& color, BorderRadius borderRadius = BorderRadius()) {
        return DrawCall(LineDrawData {
            .start = start,
            .end = end,
            .thickness = thickness,
            .color = color,
            .borderRadius = borderRadius
        });
    }

    template<size_t Size>
    inline DrawCall<SdfTextDrawData> TextSdf(sge::RichText<Size> text, glm::vec2 position, sge::TextAlignment alignment, const sge::Font& font) {
        auto copied = sge::FrameArena::Copy(text);
        return DrawCall(SdfTextDrawData {
            .sections = std::span(copied->data(), copied->size()),
            .font = font,
            .position = position,
            .alignment = alignment
        });
    }

    inline DrawCall<SdfTextDrawData> TextSdf(std::string_view text, float size, sge::LinearRgba color, glm::vec2 position, sge::TextAlignment alignment, const sge::Font& font) {
        auto section = sge::FrameArena::Allocate<RichTextSection>();
        section->text = text;
        section->color = color;
        section->size = size;

        return DrawCall(SdfTextDrawData {
            .sections = std::span(section, 1),
            .font = font,
            .position = position,
            .alignment = alignment
        });
    }

    template<size_t Size>
    inline DrawCall<VectorTextDrawData> TextVector(sge::RichText<Size> text, glm::vec2 position, sge::TextAlignment alignment, const sge::FontVector& font) {
        return DrawCall(VectorTextDrawData {
            .sections = std::span(text.data(), text.size()),
            .font = font,
            .position = position,
            .alignment = alignment
        });
    }
};

// Utility template struct for mapping a drawable item to the batch that renders it.
template<class Item> struct BatchFor;
template<> struct BatchFor<Sprite> { using type = SpriteBatch; };
template<> struct BatchFor<TextureAtlasSprite> { using type = SpriteBatch; };
template<> struct BatchFor<ShapeDrawData> { using type = ShapeBatch; };
template<> struct BatchFor<LineDrawData> { using type = LineBatch; };
template<> struct BatchFor<VectorTextDrawData> { using type = TextVectorBatch; };
template<> struct BatchFor<SdfTextDrawData> { using type = TextSdfBatch; };

/*
 * A helper class for dispatching different types of draw commands to corresponding Batch instances.
 */
class BatchGroup {
public:
    explicit BatchGroup(std::shared_ptr<Renderer2D> renderer) :
        m_renderer(std::move(renderer)) {}

    // Construct (or replace) a batch with custom ctor args.
    // The first argument (sge::Renderer) is provided automatically.
    template<class B, class... Args>
    B& Emplace(Args&&... args) {
        auto& slot = Slot<B>();
        slot = std::make_unique<B>(m_renderer, std::forward<Args>(args)...);
        return static_cast<B&>(*slot);
    }

    // Hot path: same lookup as before, creation only happens on a miss
    template<class B>
    B& Batch() {
        auto& slot = Slot<B>();
        if (!slot) [[unlikely]] {
            slot = CreateDefault<B>();
        }
        return static_cast<B&>(*slot);
    }

    template<class Item>
    uint32_t Draw(const Item& item) {
        DrawState state;
        state.scissor = GetCurrentScissor().value_or(sge::IRect());
        state.order = GetOrder();
        state.blendMode = GetCurrentBlendMode().value_or(sge::BlendMode::AlphaBlend);
        state.customFlags = std::nullopt;
        Batch<typename BatchFor<Item>::type>().Add(item, state);
        return state.order;
    }

    template<class Item>
    uint32_t Draw(const DrawCall<Item>& call) {
        DrawState state;
        state.scissor = GetCurrentScissor().value_or(sge::IRect());
        state.order = GetOrder(call.GetCustomOrder());
        state.blendMode = GetCurrentBlendMode().value_or(sge::BlendMode::AlphaBlend);
        state.customFlags = call.GetCustomFlags();

        using WithoutCvRef = std::remove_cvref_t<Item>;
        using PureType = std::remove_pointer_t<WithoutCvRef>;
        Batch<typename BatchFor<PureType>::type>().Add(call.GetItem(), state);
        return state.order;
    }

    void Submit();

    void Reset() {
        m_order = 0;
        m_order_mode = false;
        for (auto& batch : m_batches) {
            if (!batch) continue;
            batch->Reset();
        }
    }

    void BeginOrderMode(int order, bool advance) noexcept {
        m_order_mode = true;
        m_global_order.value = order < 0 ? m_order : order;
        m_global_order.advance = advance;
    }

    void BeginOrderMode(int order = -1) noexcept {
        BeginOrderMode(order, true);
    }

    void BeginOrderMode(bool advance) noexcept {
        BeginOrderMode(-1, advance);
    }

    void EndOrderMode() noexcept {
        m_order_mode = false;
        m_global_order.value = 0;
        m_global_order.advance = false;
    }

    void BeginScissorMode(sge::IRect scissor) {
        m_scissor_stack.push_back(scissor);
    }

    void EndScissorMode() {
        SGE_ASSERT_M(!m_scissor_stack.empty(), "EndScissorMode() called before BeginScissorMode()");
        m_scissor_stack.pop_back();
    }

    void BeginBlendMode(sge::BlendMode blendMode) {
        m_blend_mode_stack.push_back(blendMode);
    }

    void EndBlendMode() {
        SGE_ASSERT_M(!m_blend_mode_stack.empty(), "EndBlendMode() called before BeginBlendMode()");
        m_blend_mode_stack.pop_back();
    }

    [[nodiscard]]
    inline std::optional<sge::IRect> GetCurrentScissor() const noexcept {
        return m_scissor_stack.empty() ? std::nullopt : std::optional(m_scissor_stack.back());
    }

    [[nodiscard]]
    inline std::optional<sge::BlendMode> GetCurrentBlendMode() const noexcept {
        return m_blend_mode_stack.empty() ? std::nullopt : std::optional(m_blend_mode_stack.back());
    }

private:
    template<class B>
    std::shared_ptr<IBatch>& Slot() {
        const size_t id = internal::BatchTypeIdentifier<B>::id;
        if (id >= m_batches.size()) m_batches.resize(id + 1);
        return m_batches[id];
    }

    template<class B>
    std::shared_ptr<IBatch> CreateDefault() {
        if constexpr (std::is_constructible_v<B, Renderer2D&>) {
            return std::make_unique<B>(*m_renderer);
        } else {
            SGE_PANIC("Batch is not configured, call ctx.Emplace<B>(...) first");
        }
    }

    uint32_t GetOrder(const sge::Order customOrder = Order()) {
        const uint32_t order = m_order_mode
            ? m_global_order.value + std::max(customOrder.value, 0)
            : (customOrder.value >= 0 ? customOrder.value : m_order);

        if (customOrder.advance || m_global_order.advance)
            m_order = std::max(m_order, order + 1);

        return order;
    }

private:
    std::shared_ptr<Renderer2D> m_renderer;
    LLGL::SmallVector<std::shared_ptr<IBatch>, 6> m_batches;

    std::vector<sge::IRect> m_scissor_stack;
    std::vector<sge::BlendMode> m_blend_mode_stack;
    Order m_global_order;
    uint32_t m_order = 0;
    bool m_order_mode = false;
};

} // namespace sge

#endif
