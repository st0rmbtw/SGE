#ifndef _SGE_RENDERER_BATCH_HPP_
#define _SGE_RENDERER_BATCH_HPP_

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../types/texture.hpp"
#include "../types/sprite.hpp"
#include "../types/nine_patch.hpp"
#include "../types/rich_text.hpp"
#include "../types/order.hpp"
#include "../types/font.hpp"
#include "../types/shape.hpp"
#include "../types/color.hpp"
#include "../types/blend_mode.hpp"

#include "../defines.hpp"

_SGE_BEGIN

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
    uint32_t shape;
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

    DrawCommand(DrawCommandSprite sprite_data, uint32_t id, uint32_t order, sge::BlendMode blend_mode) :
        m_sprite_data(sprite_data),
        m_id(id),
        m_order(order),
        m_type(Type::DrawSprite),
        m_blend_mode(blend_mode) {}

    DrawCommand(DrawCommandGlyph glyph_data, uint32_t id, uint32_t order, sge::BlendMode blend_mode) :
        m_glyph_data(glyph_data),
        m_id(id),
        m_order(order),
        m_type(Type::DrawGlyph),
        m_blend_mode(blend_mode) {}

    DrawCommand(DrawCommandNinePatch ninepatch_data, uint32_t id, uint32_t order, sge::BlendMode blend_mode) :
        m_ninepatch_data(ninepatch_data),
        m_id(id),
        m_order(order),
        m_type(Type::DrawNinePatch),
        m_blend_mode(blend_mode) {}

    DrawCommand(DrawCommandShape shape_data, uint32_t id, uint32_t order, sge::BlendMode blend_mode) :
        m_shape_data(shape_data),
        m_id(id),
        m_order(order),
        m_type(Type::DrawShape),
        m_blend_mode(blend_mode) {}

    DrawCommand(DrawCommandLine line_data, uint32_t id, uint32_t order, sge::BlendMode blend_mode) :
        m_line_data(line_data),
        m_id(id),
        m_order(order),
        m_type(Type::DrawLine),
        m_blend_mode(blend_mode) {}

    [[nodiscard]] inline Type type() const { return m_type; }
    [[nodiscard]] inline uint32_t id() const { return m_id; }

    [[nodiscard]] inline uint32_t order() const { return m_order; }
    [[nodiscard]] inline sge::BlendMode blend_mode() const { return m_blend_mode; }

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

    Batch() {
        m_draw_commands.reserve(500);
        m_flush_queue.reserve(100);
    };

    Batch(const Batch&) = delete;
    Batch& operator=(const Batch&) = delete;

    Batch& operator=(Batch&& other) {
        m_draw_commands = std::move(other.m_draw_commands);
        m_flush_queue = std::move(other.m_flush_queue);

        m_sprite_data = other.m_sprite_data;
        m_glyph_data = other.m_glyph_data;
        m_ninepatch_data = other.m_ninepatch_data;
        m_shape_data = other.m_shape_data;
        m_line_data = other.m_line_data;

        m_order = other.m_order;

        m_draw_commands_done = other.m_draw_commands_done;

        m_global_order = other.m_global_order;

        m_order_mode = other.m_order_mode;
        m_depth_enabled = other.m_depth_enabled;
        m_is_ui = other.m_is_ui;

        return *this;
    }

    inline void SetDepthEnabled(bool depth_enabled) { m_depth_enabled = depth_enabled; }
    inline void SetIsUi(bool is_ui) { m_is_ui = is_ui; }

    inline void SetBlendMode(sge::BlendMode blend_mode) {
        m_blend_mode = blend_mode;
    }

    inline void BeginBlendMode(sge::BlendMode blend_mode) {
        m_prev_blend_mode = m_blend_mode;
        m_blend_mode = blend_mode;
    }

    inline void EndBlendMode() {
        m_blend_mode = m_prev_blend_mode;
    }

    inline void BeginOrderMode(int order, bool advance) {
        m_order_mode = true;
        m_global_order.value = order < 0 ? m_order : order;
        m_global_order.advance = advance;
    }

    inline void BeginOrderMode(int order = -1) {
        BeginOrderMode(order, true);
    }

    inline void BeginOrderMode(bool advance) {
        BeginOrderMode(-1, advance);
    }

    inline void EndOrderMode() {
        m_order_mode = false;
        m_global_order.value = 0;
        m_global_order.advance = false;
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

    uint32_t DrawShape(sge::Shape::Type shape, glm::vec2 position, glm::vec2 size, const sge::LinearRgba& color, const sge::LinearRgba& border_color, float border_thickness, glm::vec4 border_radius = glm::vec4(0.0f), sge::Anchor anchor = sge::Anchor::Center, sge::Order custom_order = {});

    inline uint32_t DrawCircle(glm::vec2 position, glm::vec2 size, const sge::LinearRgba& color, float border_thickness = 0.0f, const sge::LinearRgba& border_color = sge::LinearRgba::transparent(), sge::Anchor anchor = sge::Anchor::Center, sge::Order custom_order = {}) {
        return DrawShape(sge::Shape::Circle, position, size, color, border_color, border_thickness, glm::vec4(0.0), anchor, custom_order);
    }

    inline uint32_t DrawCircle(glm::vec2 position, float radius, const sge::LinearRgba& color, float border_thickness = 0.0f, const sge::LinearRgba& border_color = sge::LinearRgba::transparent(), sge::Anchor anchor = sge::Anchor::Center, sge::Order custom_order = {}) {
        return DrawShape(sge::Shape::Circle, position, glm::vec2(radius * 2.0f), color, border_color, border_thickness, glm::vec4(0.0), anchor, custom_order);
    }

    // border_radius = [topLeft, topRight, bottomLeft, bottomRight]
    inline uint32_t DrawRect(glm::vec2 position, glm::vec2 size, const sge::LinearRgba& color, float border_thickness = 0.0f, const sge::LinearRgba& border_color = sge::LinearRgba::transparent(), glm::vec4 border_radius = glm::vec4(0.0f), sge::Anchor anchor = sge::Anchor::Center, sge::Order custom_order = {}) {
        return DrawShape(sge::Shape::Rect, position, size, color, border_color, border_thickness, border_radius, anchor, custom_order);
    }

    // Angles in radians
    inline uint32_t DrawArc(glm::vec2 position, float outer_radius, float inner_radius, const sge::LinearRgba& color, float start_angle, float end_angle, sge::Anchor anchor = sge::Anchor::Center, sge::Order custom_order = {}) {
        return DrawShape(sge::Shape::Arc, position, glm::vec2(outer_radius * 2.0f), color, sge::LinearRgba(0.0f), inner_radius, glm::vec4(start_angle, end_angle, 0.0f, 0.0f), anchor, custom_order);
    }

    uint32_t DrawLine(glm::vec2 start, glm::vec2 end, float thickness, const sge::LinearRgba& color, const glm::vec4& border_radius = glm::vec4(0.0f), sge::Order custom_order = {});

    inline void Reset() {
        m_draw_commands.clear();
        m_flush_queue.clear();
        m_order = 0;

        m_sprite_data.Reset();
        m_glyph_data.Reset();
        m_ninepatch_data.Reset();
        m_shape_data.Reset();
        m_line_data.Reset();

        m_draw_commands_done = 0;
    }

    [[nodiscard]]
    inline bool DepthEnabled() const { return m_depth_enabled; }

    [[nodiscard]]
    inline bool IsUi() const { return m_is_ui; }

    [[nodiscard]]
    inline uint32_t order() const { return m_order; }

    [[nodiscard]] inline const Data& sprite_data() const { return m_sprite_data; }
    [[nodiscard]] inline const Data& glyph_data() const { return m_glyph_data; }
    [[nodiscard]] inline const Data& ninepatch_data() const { return m_ninepatch_data; }
    [[nodiscard]] inline const Data& shape_data() const { return m_shape_data; }
    [[nodiscard]] inline const Data& line_data() const { return m_line_data; }

    inline uint32_t GetNextOrder(Order custom_order = {});

private:
    uint32_t AddSpriteDrawCommand(const sge::BaseSprite& sprite, const glm::vec4& uv_offset_scale, const sge::Texture& texture, sge::Order custom_order);
    uint32_t AddNinePatchDrawCommand(const sge::NinePatch& ninepatch, const glm::vec4& uv_offset_scale, sge::Order custom_order);

    inline void set_draw_commands_done(size_t count) { m_draw_commands_done = count; }

    [[nodiscard]] inline size_t draw_commands_done() const { return m_draw_commands_done; }

    [[nodiscard]] inline const FlushQueue& flush_queue() const { return m_flush_queue; }
    [[nodiscard]] inline FlushQueue& flush_queue() { return m_flush_queue; }

    [[nodiscard]] inline const DrawCommands& draw_commands() const { return m_draw_commands; }
    [[nodiscard]] inline DrawCommands& draw_commands() { return m_draw_commands; }

    [[nodiscard]] inline Data& sprite_data() { return m_sprite_data; }
    [[nodiscard]] inline Data& glyph_data() { return m_glyph_data; }
    [[nodiscard]] inline Data& ninepatch_data() { return m_ninepatch_data; }
    [[nodiscard]] inline Data& shape_data() { return m_shape_data; }
    [[nodiscard]] inline Data& line_data() { return m_line_data; }

private:
    static constexpr size_t MAX_QUADS = 2500;
    static constexpr size_t MAX_GLYPHS = 2500;

    DrawCommands m_draw_commands;
    FlushQueue m_flush_queue;

    Data m_sprite_data;
    Data m_glyph_data;
    Data m_ninepatch_data;
    Data m_shape_data;
    Data m_line_data;

    uint32_t m_order = 0;

    uint32_t m_draw_commands_done = 0;

    sge::Order m_global_order;

    bool m_order_mode = false;
    bool m_depth_enabled = false;
    bool m_is_ui = false;

    sge::BlendMode m_prev_blend_mode = sge::BlendMode::AlphaBlend;
    sge::BlendMode m_blend_mode = sge::BlendMode::AlphaBlend;
};

_SGE_END

#endif