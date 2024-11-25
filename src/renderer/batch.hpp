#ifndef RENDER_BATCH_HPP
#define RENDER_BATCH_HPP

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <LLGL/LLGL.h>

#include "types.hpp"

#include "../types/sprite.hpp"
#include "../types/render_layer.hpp"
#include "../types/shape.hpp"
#include "../optional.hpp"

constexpr size_t MAX_QUADS = 5000;

namespace SpriteFlags {
    enum : uint8_t {
        HasTexture = 0,
        UI,
    };
};

namespace ShapeFlags {
    enum : uint8_t {
        UI = 1,
    };
};

struct SpriteData {
    glm::vec2 position;
    glm::quat rotation;
    glm::vec2 size;
    glm::vec2 offset;
    glm::vec4 uv_offset_scale;
    glm::vec4 color;
    glm::vec4 outline_color;
    float outline_thickness;
    Texture texture;
    LLGL::RenderTarget *render_to;
    uint32_t order;
    bool is_ui;
};

struct ShapeData {
    glm::vec2 position;
    glm::vec2 size;
    glm::vec2 offset;
    glm::vec4 color;
    glm::vec4 border_color;
    float border_thickness;
    uint32_t order;
    bool is_ui;
};

struct FlushData {
    Texture texture;
    LLGL::RenderTarget* render_to = nullptr;
    int offset;
    uint32_t count;
};

struct GlyphData {
    Texture texture;
    LLGL::RenderTarget* render_to = nullptr;
    glm::vec3 color;
    glm::vec2 pos;
    glm::vec2 size;
    glm::vec2 tex_size;
    glm::vec2 tex_uv;
    uint32_t order;
    bool is_ui;
};

class RenderBatch {
public:
    virtual void init() = 0;
    virtual void render() = 0;

    virtual void begin() = 0;
    virtual void terminate() = 0;

protected:
    LLGL::Buffer* m_vertex_buffer = nullptr;
    LLGL::Buffer* m_instance_buffer = nullptr;
    LLGL::BufferArray* m_buffer_array = nullptr;

    LLGL::PipelineState* m_pipeline = nullptr;
};

class RenderBatchShape : public RenderBatch {
public:
    void draw_shape(Shape::Type shape, glm::vec2 position, glm::vec2 size, const glm::vec4& color, const glm::vec4& border_color, float border_thickness, float border_radius, Anchor anchor, bool is_ui, int depth);
    void init() override;
    void render() override;
    void begin() override;
    void terminate() override;
private:
    ShapeInstance* m_buffer = nullptr;
    ShapeInstance* m_buffer_ptr = nullptr;

    size_t m_count = 0;
};

class RenderBatchSprite : public RenderBatch {
public:
    void draw_sprite(const BaseSprite& sprite, const glm::vec4& uv_offset_scale, const tl::optional<Texture>& sprite_texture, RenderLayer layer, bool is_ui, int depth, LLGL::RenderTarget* render_target = nullptr);
    void init() override;
    void render() override;
    void begin() override;
    void terminate() override;
private:
    void flush();
private:
    std::vector<SpriteData> m_sprites;
    std::vector<FlushData> m_sprite_flush_queue;

    SpriteInstance* m_buffer = nullptr;
    SpriteInstance* m_buffer_ptr = nullptr;
};

class RenderBatchGlyph : public RenderBatch {
public:
    void draw_glyph(const glm::vec2& pos, const glm::vec2& size, const glm::vec3& color, const Texture& font_texture, const glm::vec2& tex_uv, const glm::vec2& tex_size, bool ui, uint32_t depth, LLGL::RenderTarget* render_to);

    void init() override;
    void render() override;
    void begin() override;
    void terminate() override;

    [[nodiscard]] inline bool is_empty() const { return m_glyphs.empty(); }
private:
    void flush();
private:
    std::vector<GlyphData> m_glyphs;
    std::vector<FlushData> m_glyphs_flush_queue;

    GlyphInstance* m_buffer = nullptr;
    GlyphInstance* m_buffer_ptr = nullptr;
};

#endif
