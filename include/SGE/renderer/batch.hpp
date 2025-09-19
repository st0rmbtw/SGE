#ifndef _SGE_RENDERER_BATCH_HPP_
#define _SGE_RENDERER_BATCH_HPP_

#include <vector>

#include <LLGL/RenderSystem.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <SGE/math/rect.hpp>
#include <SGE/types/texture.hpp>
#include <SGE/types/sprite.hpp>
#include <SGE/types/nine_patch.hpp>
#include <SGE/types/rich_text.hpp>
#include <SGE/types/order.hpp>
#include <SGE/types/font.hpp>
#include <SGE/types/shape.hpp>
#include <SGE/types/color.hpp>
#include <SGE/types/blend_mode.hpp>
#include <SGE/defines.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/macros.hpp>

namespace sge {

class Renderer;

namespace internal {

enum class FlushDataType : uint8_t {
    Sprite = 0,
    Glyph,
    NinePatch,
    Shape,
    Line,
};

struct FlushData {
    std::optional<sge::Texture> texture;
    sge::IRect scissor;
    uint32_t offset;
    uint32_t count;
    uint32_t order;
    FlushDataType type;
    sge::BlendMode blend_mode;
};

struct DrawCommandSprite {
    sge::Texture texture;
    glm::quat rotation;
    glm::vec4 uv_offset_scale;
    glm::vec4 color;
    glm::vec4 outline_color;
    glm::vec3 position;
    glm::vec2 size;
    glm::vec2 offset;
    float outline_thickness;
    bool ignore_camera_zoom;
    bool depth_enabled;
};

struct DrawCommandNinePatch {
    sge::Texture texture;
    glm::quat rotation;
    glm::vec4 uv_offset_scale;
    glm::vec4 color;
    glm::uvec4 margin;
    glm::vec2 position;
    glm::vec2 size;
    glm::vec2 offset;
    glm::vec2 source_size;
    glm::vec2 output_size;
};

struct DrawCommandGlyph {
    sge::Texture texture;
    glm::vec3 color;
    glm::vec2 pos;
    glm::vec2 size;
    glm::vec2 tex_size;
    glm::vec2 tex_uv;
};

struct DrawCommandShape {
    glm::vec2 position;
    glm::vec2 size;
    glm::vec2 offset;
    sge::LinearRgba color;
    sge::LinearRgba border_color;
    glm::vec4 border_radius;
    float border_thickness;
    uint8_t shape;
};

struct DrawCommandLine {
    glm::vec2 start;
    glm::vec2 end;
    sge::LinearRgba color;
    glm::vec4 border_radius;
    float thickness;
};

class DrawCommand {
public:
    enum Type : uint8_t {
        DrawSprite = 0,
        DrawGlyph,
        DrawNinePatch,
        DrawShape,
        DrawLine
    };

    DrawCommand(DrawCommandSprite sprite_data, sge::Rect scissor, uint32_t id, uint32_t order, sge::BlendMode blend_mode) :
        m_sprite_data(sprite_data),
        m_scissor(scissor),
        m_id(id),
        m_order(order),
        m_type(Type::DrawSprite),
        m_blend_mode(blend_mode) {}

    DrawCommand(DrawCommandGlyph glyph_data, sge::Rect scissor, uint32_t id, uint32_t order, sge::BlendMode blend_mode) :
        m_glyph_data(glyph_data),
        m_scissor(scissor),
        m_id(id),
        m_order(order),
        m_type(Type::DrawGlyph),
        m_blend_mode(blend_mode) {}

    DrawCommand(DrawCommandNinePatch ninepatch_data, sge::Rect scissor, uint32_t id, uint32_t order, sge::BlendMode blend_mode) :
        m_ninepatch_data(ninepatch_data),
        m_scissor(scissor),
        m_id(id),
        m_order(order),
        m_type(Type::DrawNinePatch),
        m_blend_mode(blend_mode) {}

    DrawCommand(DrawCommandShape shape_data, sge::Rect scissor, uint32_t id, uint32_t order, sge::BlendMode blend_mode) :
        m_shape_data(shape_data),
        m_scissor(scissor),
        m_id(id),
        m_order(order),
        m_type(Type::DrawShape),
        m_blend_mode(blend_mode) {}

    DrawCommand(DrawCommandLine line_data, sge::Rect scissor, uint32_t id, uint32_t order, sge::BlendMode blend_mode) :
        m_line_data(line_data),
        m_scissor(scissor),
        m_id(id),
        m_order(order),
        m_type(Type::DrawLine),
        m_blend_mode(blend_mode) {}

    [[nodiscard]] inline Type type() const { return m_type; }
    [[nodiscard]] inline uint32_t id() const { return m_id; }

    [[nodiscard]] inline uint32_t order() const { return m_order; }
    [[nodiscard]] inline sge::BlendMode blend_mode() const { return m_blend_mode; }

    [[nodiscard]] inline sge::IRect scissor() const { return m_scissor; }

    [[nodiscard]] inline const sge::Texture* texture() const {
        switch (m_type) {
        case DrawSprite: return &m_sprite_data.texture;
        case DrawGlyph: return &m_glyph_data.texture;
        case DrawNinePatch: return &m_ninepatch_data.texture;
        default: return nullptr;
        }
    }

    [[nodiscard]] inline const DrawCommandSprite& sprite_data() const { return m_sprite_data; }
    [[nodiscard]] inline const DrawCommandGlyph& glyph_data() const { return m_glyph_data; }
    [[nodiscard]] inline const DrawCommandNinePatch& ninepatch_data() const { return m_ninepatch_data; }
    [[nodiscard]] inline const DrawCommandShape& shape_data() const { return m_shape_data; }
    [[nodiscard]] inline const DrawCommandLine& line_data() const { return m_line_data; }

private:
    union {
        DrawCommandNinePatch m_ninepatch_data;
        DrawCommandSprite m_sprite_data;
        DrawCommandGlyph m_glyph_data;
        DrawCommandShape m_shape_data;
        DrawCommandLine m_line_data;
    };

    sge::IRect m_scissor;

    uint32_t m_id;
    uint32_t m_order;

    Type m_type;
    sge::BlendMode m_blend_mode;
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

};

struct BatchDesc {
    /**
     * @brief Custom sprite fragment shader
     * 
     * @note If null, the default sprite fragment shader is used
     */
    LLGL::Shader* sprite_shader = nullptr;
    /**
     * @brief Custom font fragment shader
     * 
     * @note If null, the default font fragment shader is used
     */
    LLGL::Shader* font_shader = nullptr;
    bool enable_scissor = false;
};

class Batch {
    friend class sge::Renderer;

public:
    struct Data {
        void Reset() {
            offset = 0;
            count = 0;
        }

        uint32_t offset = 0;
        uint32_t count = 0;
    };

    using DrawCommands = std::vector<internal::DrawCommand>;
    using FlushQueue = std::vector<internal::FlushData>;

    Batch(Renderer& renderer, const BatchDesc& desc);

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

    uint32_t DrawText(const sge::RichTextSection* sections, size_t size, const glm::vec2& position, const sge::Font& font, sge::Order order = {});

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
        return DrawShape(sge::Shape::Circle, position, glm::vec2(circle.radius * 2.0f), circle.color, circle.border_color, circle.border_thickness, glm::vec4(0.0), circle.anchor, custom_order);
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
        return DrawShape(sge::Shape::Arc, position, glm::vec2(arc.outer_radius * 2.0f), arc.color, sge::LinearRgba(0.0f), arc.inner_radius, glm::vec4(arc.start_angle, arc.end_angle, 0.0f, 0.0f), arc.anchor, custom_order);
    }

    inline uint32_t DrawArc(glm::vec2 position, const ShapeArc& arc) {
        return DrawArc(position, sge::Order(), arc);
    }

    uint32_t DrawLine(glm::vec2 start, glm::vec2 end, float thickness, const sge::LinearRgba& color, const glm::vec4& border_radius = glm::vec4(0.0f), sge::Order custom_order = {});

    inline void Reset() {
        m_draw_commands.clear();
        m_flush_queue.clear();
        m_scissors.clear();
        m_order = 0;
        
        m_sprite_data.Reset();
        m_glyph_data.Reset();
        m_ninepatch_data.Reset();
        m_shape_data.Reset();
        m_line_data.Reset();
        
        m_order_mode = false;

        m_draw_commands_done = 0;
    }

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
    inline LLGL::PipelineState* NinepatchPipeline() const noexcept {
        return m_ninepatch_pipeline;
    }

    [[nodiscard]]
    inline LLGL::PipelineState* GlyphPipeline() const noexcept {
        return m_glyph_pipeline;
    }

    [[nodiscard]]
    inline LLGL::PipelineState* ShapePipeline() const noexcept {
        return m_shape_pipeline;
    }

    [[nodiscard]]
    inline LLGL::PipelineState* LinePipeline() const noexcept {
        return m_line_pipeline;
    }

    inline uint32_t GetOrder(sge::Order custom_order = {});

    void Destroy(const LLGL::RenderSystemPtr& context) {
        m_sprite_pipeline.Destroy(context);
        
        SGE_RESOURCE_RELEASE(m_ninepatch_pipeline);
        SGE_RESOURCE_RELEASE(m_glyph_pipeline);
        SGE_RESOURCE_RELEASE(m_shape_pipeline);
        SGE_RESOURCE_RELEASE(m_line_pipeline);
    }

private:
    uint32_t DrawShape(sge::Shape::Type shape, glm::vec2 position, glm::vec2 size, const sge::LinearRgba& color, const sge::LinearRgba& border_color, float border_thickness, glm::vec4 border_radius = glm::vec4(0.0f), sge::Anchor anchor = sge::Anchor::Center, sge::Order custom_order = {});

    uint32_t AddSpriteDrawCommand(const sge::BaseSprite& sprite, const glm::vec4& uv_offset_scale, const sge::Texture& texture, sge::Order custom_order);
    uint32_t AddNinePatchDrawCommand(const sge::NinePatch& ninepatch, const glm::vec4& uv_offset_scale, sge::Order custom_order);

    inline void set_draw_commands_done(size_t count) { m_draw_commands_done = count; }

    [[nodiscard]]
    inline size_t draw_commands_done() const noexcept {
        return m_draw_commands_done;
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
    inline const DrawCommands& draw_commands() const noexcept {
        return m_draw_commands;
    }

    [[nodiscard]]
    inline DrawCommands& draw_commands() noexcept {
        return m_draw_commands;
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

private:
    static constexpr size_t MAX_QUADS = 2500;
    static constexpr size_t MAX_GLYPHS = 2500;

    SpriteBatchPipeline m_sprite_pipeline{};

    DrawCommands m_draw_commands;
    FlushQueue m_flush_queue;

    std::vector<sge::IRect> m_scissors;

    Data m_sprite_data;
    Data m_glyph_data;
    Data m_ninepatch_data;
    Data m_shape_data;
    Data m_line_data;

    LLGL::PipelineState* m_ninepatch_pipeline = nullptr;
    LLGL::PipelineState* m_glyph_pipeline = nullptr;
    LLGL::PipelineState* m_shape_pipeline = nullptr;
    LLGL::PipelineState* m_line_pipeline = nullptr;

    uint32_t m_order = 0;

    uint32_t m_draw_commands_done = 0;

    sge::Order m_global_order;

    bool m_order_mode = false;
    bool m_depth_enabled = false;
    bool m_is_ui = false;
    bool m_scissor_enabled = false;

    sge::BlendMode m_prev_blend_mode = sge::BlendMode::AlphaBlend;
    sge::BlendMode m_blend_mode = sge::BlendMode::AlphaBlend;
};

}

#endif