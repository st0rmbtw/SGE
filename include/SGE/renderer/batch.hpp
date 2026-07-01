#ifndef _SGE_RENDERER_BATCH_HPP_
#define _SGE_RENDERER_BATCH_HPP_

#include <vector>

#include <LLGL/RenderSystem.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <SGE/defines.hpp>
#include <SGE/math/rect.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/types/blend_mode.hpp>
#include <SGE/types/color.hpp>
#include <SGE/types/font.hpp>
#include <SGE/types/nine_patch.hpp>
#include <SGE/types/order.hpp>
#include <SGE/types/rich_text.hpp>
#include <SGE/types/shape.hpp>
#include <SGE/types/sprite.hpp>
#include <SGE/types/texture.hpp>
#include <SGE/utils/containers/heaparray.hpp>

namespace sge {

class Renderer2D;

namespace internal {

enum class FlushDataType : uint8_t {
    Sprite = 0,
    Glyph,
    NinePatch,
    Shape,
    Line
};

struct BatchTexture {
    LLGL::Texture* ptr = nullptr;
    LLGL::Sampler* sampler = nullptr;
    int id = 0;

    [[nodiscard]] inline bool is_valid() const { return id >= 0 && ptr != nullptr; }

    bool operator==(const BatchTexture& other) const {
        return id == other.id;
    }
};

struct BatchSimpleState {
    sge::IRect scissor;
    uint32_t order;
    sge::BlendMode blend_mode;

    bool operator==(const BatchSimpleState& other) const {
        return scissor == other.scissor 
            && order == other.order 
            && blend_mode == other.blend_mode;
    }
};

struct BatchTextureState {
    BatchTexture texture;
    sge::IRect scissor;
    uint32_t order;
    sge::BlendMode blend_mode;

    bool operator==(const BatchTextureState& other) const {
        return texture == other.texture
            && scissor == other.scissor 
            && order == other.order 
            && blend_mode == other.blend_mode;
    }
};

struct BatchGlyphState {
    sge::Ref<LLGL::Buffer> buffer;
    sge::IRect scissor;
    uint32_t order;
    sge::BlendMode blend_mode;

    bool operator==(const BatchGlyphState& other) const {
        return buffer == other.buffer
            && scissor == other.scissor 
            && order == other.order 
            && blend_mode == other.blend_mode;
    }
};

struct FlushData {
    union {
        BatchTexture texture;
        LLGL::Buffer* buffer;
    };
    sge::IRect scissor;
    uint32_t offset;
    uint32_t count;
    FlushDataType type;
    sge::BlendMode blend_mode;
};

struct DrawCommandSprite {
    BatchTextureState state = {};
    glm::quat rotation;
    glm::vec4 uv_offset_scale;
    glm::vec4 color;
    glm::vec4 outline_color;
    glm::vec3 position;
    glm::vec2 size;
    glm::vec2 offset;
    float outline_thickness;
};

struct DrawCommandNinePatch {
    BatchTextureState state = {};
    glm::quat rotation;
    glm::vec4 uv_offset_scale;
    glm::vec4 color;
    glm::uvec4 margin;
    glm::vec2 position;
    glm::vec2 offset;
    glm::vec2 source_size;
    glm::vec2 output_size;
};

struct DrawCommandGlyph {
    BatchGlyphState state = {};
    glm::vec3 color;
    glm::vec2 pos;
    glm::vec2 size;
    glm::vec2 em_size;
    float font_size;
    size_t offset;
    size_t count;
};

struct DrawCommandShape {
    BatchSimpleState state;
    sge::LinearRgba color;
    sge::LinearRgba border_color;
    glm::vec4 border_radius;
    glm::vec2 position;
    glm::vec2 size;
    glm::vec2 offset;
    float border_thickness;
    uint8_t shape;
};

struct DrawCommandLine {
    BatchSimpleState state;
    sge::LinearRgba color;
    glm::vec4 border_radius;
    glm::vec2 start;
    glm::vec2 end;
    float thickness;
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

inline bool SortSimpleBatchState(const BatchSimpleState& a, const BatchSimpleState& b) {
    if (a.order < b.order) return true;
    if (a.order > b.order) return false;

    const int a_scissor_size = a.scissor.width() + a.scissor.height();
    const int b_scissor_size = b.scissor.width() + b.scissor.height();

    if (a_scissor_size < b_scissor_size)
        return true;

    if (a_scissor_size > b_scissor_size)
        return false;

    uint8_t a_bm = static_cast<uint8_t>(a.blend_mode);
    uint8_t b_bm = static_cast<uint8_t>(b.blend_mode);

    if (a_bm < b_bm) return true;
    if (a_bm > b_bm) return false;

    return false;
}

inline bool SortTextureBatchState(const BatchTextureState& a, const BatchTextureState& b) {
    if (a.order < b.order) return true;
    if (a.order > b.order) return false;

    const int a_scissor_size = a.scissor.width() + a.scissor.height();
    const int b_scissor_size = b.scissor.width() + b.scissor.height();

    if (a_scissor_size < b_scissor_size)
        return true;

    if (a_scissor_size > b_scissor_size)
        return false;

    if (a.texture.id < b.texture.id) return true;
    if (a.texture.id > b.texture.id) return false;

    uint8_t a_bm = static_cast<uint8_t>(a.blend_mode);
    uint8_t b_bm = static_cast<uint8_t>(b.blend_mode);

    if (a_bm < b_bm) return true;
    if (a_bm > b_bm) return false;

    return false;
}

} // namespace internal

struct BatchDesc {
    /**
     * @brief Custom sprite fragment shader
     * 
     * @note If null, the default sprite fragment shader is used
     */
    Ref<LLGL::Shader> sprite_shader = nullptr;
    /**
     * @brief Custom font fragment shader
     * 
     * @note If null, the default font fragment shader is used
     */
    Ref<LLGL::Shader> font_shader = nullptr;

    bool enable_scissor = false;
};

class Batch {
    friend class sge::Renderer2D;

public:
    struct Data {
        void Reset() {
            offset = 0;
            total_count = 0;
        }

        uint32_t offset = 0;
        uint32_t total_count = 0;
    };

    using FlushQueue = std::vector<internal::FlushData>;

    Batch(Renderer2D& renderer, const BatchDesc& desc);

    Batch(const Batch&) = delete;
    Batch& operator=(const Batch&) = delete;

    Batch(Batch&&) = default;
    Batch& operator=(Batch&&) = default;

    inline void SetDepthEnabled(bool depth_enabled) noexcept {
        m_depth_enabled = depth_enabled;
    }

    inline void SetIsUi(bool is_ui) noexcept {
        m_is_ui = is_ui;
    }

    inline void SetBlendMode(sge::BlendMode blend_mode) noexcept {
        m_blend_mode = blend_mode;
    }

    inline void SetMaxCount(uint32_t max_count) noexcept {
        m_max_count = max_count;
    }

    inline void BeginBlendMode(sge::BlendMode blend_mode) noexcept {
        m_prev_blend_mode = m_blend_mode;
        m_blend_mode = blend_mode;
    }

    inline void EndBlendMode() noexcept {
        m_blend_mode = m_prev_blend_mode;
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

    inline void BeginScissorMode(sge::Rect area) noexcept {
        m_scissors.emplace_back(area);
    }

    inline void EndScissorMode() noexcept {
        if (m_scissors.empty()) return;
        m_scissors.pop_back();
    }

    uint32_t DrawTextVector(const sge::RichTextSection* sections, size_t size, glm::vec2 position, const sge::FontVector& font, sge::Order order = {});

    template <size_t Size>
    inline uint32_t DrawTextVector(const sge::RichText<Size>& text, glm::vec2 position, const sge::FontVector& font, sge::Order order = {}) {
        return DrawTextVector(text.sections, Size, position, font, order);
    }

    uint32_t DrawText(const sge::RichTextSection* sections, size_t size, glm::vec2 position, const sge::Font& font, sge::Order order = {});

    template <size_t Size>
    inline uint32_t DrawText(const sge::RichText<Size>& text, glm::vec2 position, const sge::Font& font, sge::Order order = {}) {
        return DrawText(text.sections, Size, position, font, order);
    }

    uint32_t DrawAtlasSprite(const sge::TextureAtlasSprite& sprite, sge::Order order = {});

    inline uint32_t DrawSprite(const sge::Sprite& sprite, sge::Order custom_order = {}) {
        const glm::vec4 uv_offset_scale = internal::get_uv_offset_scale(sprite.flip_x(), sprite.flip_y());
        return AddSpriteDrawCommand(sprite, uv_offset_scale, sprite.texture(), custom_order);
    }

    inline uint32_t DrawNinePatch(const sge::NinePatch& ninepatch, sge::Order custom_order = {}) {
        const glm::vec4 uv_offset_scale = internal::get_uv_offset_scale(ninepatch.flip_x(), ninepatch.flip_y());
        return AddNinePatchDrawCommand(ninepatch, uv_offset_scale, custom_order);
    }

    inline uint32_t DrawCircle(glm::vec2 position, sge::Order custom_order, const ShapeCircle& circle) {
        return DrawShape(sge::Shape::Circle, position, glm::vec2(circle.radius * 2.0f), circle.color, circle.border_color, circle.border_thickness, BorderRadius::Absolute(0.0), circle.anchor, custom_order);
    }

    inline uint32_t DrawCircle(glm::vec2 position, const ShapeCircle& circle) {
        return DrawCircle(position, sge::Order(), circle);
    }

    inline uint32_t DrawRect(glm::vec2 position, sge::Order custom_order, const ShapeRect& rect) {
        return DrawShape(sge::Shape::Rect, position, rect.size, rect.color, rect.border_color, rect.border_thickness, rect.border_radius, rect.anchor, custom_order);
    }

    inline uint32_t DrawRect(glm::vec2 position, const ShapeRect& rect) {
        return DrawRect(position, sge::Order(), rect);
    }

    inline uint32_t DrawArc(glm::vec2 position, sge::Order custom_order, const ShapeArc& arc) {
        return DrawShape(sge::Shape::Arc, position, glm::vec2(arc.outer_radius * 2.0f), arc.color, sge::LinearRgba(0.0f), arc.inner_radius, BorderRadius::Absolute(arc.start_angle, arc.end_angle, 0.0f, 0.0f), arc.anchor, custom_order);
    }

    inline uint32_t DrawArc(glm::vec2 position, const ShapeArc& arc) {
        return DrawArc(position, sge::Order(), arc);
    }

    uint32_t DrawLine(glm::vec2 start, glm::vec2 end, float thickness, const sge::LinearRgba& color, BorderRadius border_radius = BorderRadius(), sge::Order custom_order = {});

    inline void Reset() {
        m_sprite_draw_commands.clear();
        m_glyph_draw_commands.clear();
        m_ninepatch_draw_commands.clear();
        m_shape_draw_commands.clear();
        m_line_draw_commands.clear();
        
        m_flush_queue.clear();
        m_scissors.clear();
        
        m_sprite_data.Reset();
        m_glyph_data.Reset();
        m_ninepatch_data.Reset();
        m_shape_data.Reset();
        m_line_data.Reset();
        
        m_order = 0;
        m_order_mode = false;
    }

    uint32_t GetOrder(sge::Order custom_order = {});

    [[nodiscard]]
    inline bool DepthEnabled() const noexcept {
        return m_depth_enabled;
    }

    [[nodiscard]]
    inline bool IsUi() const noexcept {
        return m_is_ui;
    }
    
    [[nodiscard]]
    inline bool ScissorEnabled() const noexcept {
        return m_scissor_enabled;
    }

    [[nodiscard]]
    inline uint32_t MaxCount() const noexcept {
        return m_max_count;
    }

    [[nodiscard]]
    inline uint32_t Order() const noexcept {
        return m_order;
    }

    [[nodiscard]]
    inline const Data& SpriteData() const noexcept {
        return m_sprite_data;
    }

    [[nodiscard]]
    inline const Data& GlyphData() const noexcept {
        return m_glyph_data;
    }

    [[nodiscard]]
    inline const Data& NinepatchData() const noexcept {
        return m_ninepatch_data;
    }

    [[nodiscard]]
    inline const Data& ShapeData() const noexcept {
        return m_shape_data;
    }

    [[nodiscard]]
    inline const Data& LineData() const noexcept {
        return m_line_data;
    }

    [[nodiscard]]
    inline const SpriteBatchPipeline& SpritePipeline() const noexcept {
        return m_sprite_pipeline;
    }
    
    [[nodiscard]]
    inline sge::Handle<LLGL::PipelineState> NinepatchPipeline() const noexcept {
        return m_ninepatch_pipeline;
    }

    [[nodiscard]]
    inline sge::Handle<LLGL::PipelineState> GlyphPipeline() const noexcept {
        return m_glyph_pipeline;
    }

    [[nodiscard]]
    inline sge::Handle<LLGL::PipelineState> ShapePipeline() const noexcept {
        return m_shape_pipeline;
    }

    [[nodiscard]]
    inline sge::Handle<LLGL::PipelineState> LinePipeline() const noexcept {
        return m_line_pipeline;
    }
private:
    uint32_t DrawShape(sge::Shape::Type shape, glm::vec2 position, glm::vec2 size, const sge::LinearRgba& color, const sge::LinearRgba& border_color, float border_thickness, BorderRadius border_radius = BorderRadius(), sge::Anchor anchor = sge::Anchor::Center, sge::Order custom_order = {});

    uint32_t AddSpriteDrawCommand(const sge::BaseSprite& sprite, const glm::vec4& uv_offset_scale, const sge::Texture& texture, sge::Order custom_order);
    uint32_t AddNinePatchDrawCommand(const sge::NinePatch& ninepatch, const glm::vec4& uv_offset_scale, sge::Order custom_order);

    [[nodiscard]]
    inline bool empty() const noexcept {
        return (
            m_sprite_draw_commands.empty() &&
            m_glyph_draw_commands.empty() &&
            m_ninepatch_draw_commands.empty() &&
            m_shape_draw_commands.empty() &&
            m_line_draw_commands.empty()
        );
    }

    [[nodiscard]]
    inline const FlushQueue& flush_queue() const noexcept {
        return m_flush_queue;
    }

    [[nodiscard]]
    inline FlushQueue& flush_queue() noexcept {
        return m_flush_queue;
    }

    [[nodiscard]]
    std::vector<internal::DrawCommandSprite>& sprite_draw_commands() noexcept {
        return m_sprite_draw_commands;
    }

    [[nodiscard]]
    std::vector<internal::DrawCommandGlyph>& glyph_draw_commands() noexcept {
        return m_glyph_draw_commands;
    }

    [[nodiscard]]
    std::vector<internal::DrawCommandNinePatch>& ninepatch_draw_commands() noexcept {
        return m_ninepatch_draw_commands;
    }

    [[nodiscard]]
    std::vector<internal::DrawCommandShape>& shape_draw_commands() noexcept {
        return m_shape_draw_commands;
    }

    [[nodiscard]]
    std::vector<internal::DrawCommandLine>& line_draw_commands() noexcept {
        return m_line_draw_commands;
    }

    [[nodiscard]]
    inline Data& sprite_data() noexcept {
        return m_sprite_data;
    }
    
    [[nodiscard]]
    inline Data& glyph_data() noexcept {
        return m_glyph_data;
    }

    [[nodiscard]]
    inline Data& ninepatch_data() noexcept {
        return m_ninepatch_data;
    }

    [[nodiscard]]
    inline Data& shape_data() noexcept {
        return m_shape_data;
    }
    
    [[nodiscard]]
    inline Data& line_data() noexcept {
        return m_line_data;
    }

    [[nodiscard]]
    inline size_t sprites_done() const noexcept {
        return m_sprite_draw_commands.size() - m_sprite_data.total_count;
    }

    [[nodiscard]]
    inline size_t ninepatches_done() const noexcept {
        return m_ninepatch_draw_commands.size() - m_ninepatch_data.total_count;
    }

    [[nodiscard]]
    inline size_t glyphs_done() const noexcept {
        return m_glyph_draw_commands.size() - m_glyph_data.total_count;
    }

    [[nodiscard]]
    inline size_t shapes_done() const noexcept {
        return m_shape_draw_commands.size() - m_shape_data.total_count;
    }

    [[nodiscard]]
    inline size_t lines_done() const noexcept {
        return m_line_draw_commands.size() - m_line_data.total_count;
    }

    [[nodiscard]]
    inline size_t total_remaining() const noexcept {
        return m_sprite_data.total_count
            + m_glyph_data.total_count
            + m_ninepatch_data.total_count
            + m_shape_data.total_count
            + m_line_data.total_count;
    }

private:
    static constexpr size_t MAX_QUADS = 2500;
    static constexpr size_t MAX_GLYPHS = 2500;

    SpriteBatchPipeline m_sprite_pipeline{};

    std::vector<internal::DrawCommandSprite> m_sprite_draw_commands;
    std::vector<internal::DrawCommandGlyph> m_glyph_draw_commands;
    std::vector<internal::DrawCommandNinePatch> m_ninepatch_draw_commands;
    std::vector<internal::DrawCommandShape> m_shape_draw_commands;
    std::vector<internal::DrawCommandLine> m_line_draw_commands;

    FlushQueue m_flush_queue;

    std::vector<sge::IRect> m_scissors;

    Data m_sprite_data;
    Data m_glyph_data;
    Data m_ninepatch_data;
    Data m_shape_data;
    Data m_line_data;

    sge::Handle<LLGL::PipelineState> m_ninepatch_pipeline;
    sge::Handle<LLGL::PipelineState> m_glyph_pipeline;
    sge::Handle<LLGL::PipelineState> m_shape_pipeline;
    sge::Handle<LLGL::PipelineState> m_line_pipeline;

    uint32_t m_order = 0;

    uint32_t m_max_count = UINT32_MAX;

    sge::Order m_global_order;

    bool m_order_mode = false;
    bool m_depth_enabled = false;
    bool m_is_ui = false;
    bool m_scissor_enabled = false;

    sge::BlendMode m_prev_blend_mode = sge::BlendMode::AlphaBlend;
    sge::BlendMode m_blend_mode = sge::BlendMode::AlphaBlend;
};

} // namespace sge

#endif