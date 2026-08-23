#include <SGE/profile.hpp>
#include <SGE/renderer/attributes.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/buffer_pool.hpp>
#include <SGE/renderer/material.hpp>
#include <SGE/renderer/mesh.hpp>
#include <SGE/renderer/renderer2d.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/utils.hpp>
#include <SGE/types/binding_layout.hpp>
#include <SGE/utils/utf8.hpp>

#include <LLGL/ShaderFlags.h>

#include "shaders.hpp"

namespace SpriteFlags {
    enum : uint8_t {
        UI = 0,
    };
};

namespace GlyphFlags {
    enum : uint8_t {
        UI = 0,
    };
};

namespace NinePatchFlags {
    enum : uint8_t {
        UI = 0,
    };
};

namespace ShapeFlags {
    enum : uint8_t {
        UI = 0,
    };
};

sge::Batch::Batch(Renderer2D& renderer, const BatchDesc& desc) :
    m_scissor_enabled(desc.enable_scissor)
{
    // m_sprite_draw_commands.reserve(500);
    // m_glyph_vector_draw_commands.reserve(500);
    // m_glyph_sdf_draw_commands.reserve(500);
    // m_ninepatch_draw_commands.reserve(500);
    // m_shape_draw_commands.reserve(500);
    // m_line_draw_commands.reserve(500);
    // m_flush_queue.reserve(100);
}

uint32_t sge::Batch::DrawAtlasSprite(const TextureAtlasSprite& sprite, struct Order custom_order) {
    ZoneScoped;

    const sge::Rect& rect = sprite.atlas().get_rect(sprite.index());

    glm::vec4 uv_offset_scale = glm::vec4(
        rect.min.x / sprite.atlas().texture().width(),
        rect.min.y / sprite.atlas().texture().height(),
        rect.size().x / sprite.atlas().texture().width(),
        rect.size().y / sprite.atlas().texture().height()
    );

    if (sprite.flip_x()) {
        uv_offset_scale.x += uv_offset_scale.z;
        uv_offset_scale.z *= -1.0f;
    }

    if (sprite.flip_y()) {
        uv_offset_scale.y += uv_offset_scale.w;
        uv_offset_scale.w *= -1.0f;
    }

    return AddSpriteDrawCommand(sprite, uv_offset_scale, sprite.atlas().texture(), custom_order);
}

uint32_t sge::Batch::GetOrder(sge::Order custom_order) {
    const uint32_t order = m_order_mode
        ? m_global_order.value + std::max(custom_order.value, 0)
        : (custom_order.value >= 0 ? custom_order.value : m_order);

    custom_order.advance |= m_global_order.advance;

    if (custom_order.advance) {
        m_order = std::max(m_order, order + 1);
    }

    return order;
}

uint32_t sge::Batch::DrawTextVector(const RichTextSection* sections, size_t size, glm::vec2 position, const FontVector& font, struct Order custom_order) {
    ZoneScoped;

    float x = position.x;
    float y = position.y;

    const uint32_t order = GetOrder(custom_order);
    const sge::IRect scissor = !m_scissors.empty() ? m_scissors.back() : sge::IRect();

    // for (size_t i = 0; i < size; ++i) {
    //     const RichTextSection section = sections[i];
    //     const char* str = section.text.data();
    //     const size_t length = section.text.size();
    //     const float scale = section.size / font.units_per_em;
    //     const float height = (font.ascender - font.descender) * scale;

    //     const glm::vec3 color = section.color.to_vec3();

    //     uint32_t codepoint = 0;
    //     for (size_t i = 0; i < length;) {
    //         i += utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(str) + i, codepoint);

    //         if (codepoint == '\n') {
    //             y += height;
    //             x = position.x;
    //             continue;
    //         }

    //         auto it = font.glyphs.find(codepoint);
    //         if (it == font.glyphs.end()) {
    //             it = font.glyphs.find(0);
    //         }

    //         const sge::Glyph& ch = it->second;

    //         if (codepoint == ' ') {
    //             x += ch.advance * scale;
    //             continue;
    //         }

    //         const float xpos = x + ch.bearing.x * scale;
    //         const float ypos = y + height - ch.bearing.y * scale;
    //         const glm::vec2 pos = glm::vec2(xpos, ypos);
    //         const glm::vec2 size = glm::vec2(ch.size) * scale;

    //         const auto command = internal::DrawCommandGlyphVector {
    //             .state = internal::BatchGlyphVectorState {
    //                 .curve_buffer = font.curve_buffer,
    //                 .partition_buffer = font.partition_buffer,
    //                 .scissor = scissor,
    //                 .order = order,
    //                 .blend_mode = m_blend_mode
    //             },
    //             .color = color,
    //             .pos = pos,
    //             .size = size,
    //             .em_size = ch.size,
    //             .font_size = section.size,
    //             .partition_offset = ch.data.vector.partition_offset,
    //             .partition_count = ch.data.vector.partition_count,
    //         };

    //         m_glyph_vector_draw_commands.push_back(command);

    //         ++m_glyph_vector_data.total_count;

    //         x += ch.advance * scale;
    //     }
    // }

    return order;
}

uint32_t sge::Batch::DrawText(const RichTextSection* sections, size_t size, glm::vec2 position, const Font& font, struct Order custom_order) {
    ZoneScoped;

    float x = position.x;
    float y = position.y;

    const uint32_t order = GetOrder(custom_order);
    // const sge::IRect scissor = !m_scissors.empty() ? m_scissors.back() : sge::IRect();

    // const auto texture = internal::BatchTexture {
    //     .ptr = font.texture.internal().Get(),
    //     .sampler = font.texture.sampler()->internal().Get(),
    //     .id = font.texture.id()
    // };

    // for (size_t i = 0; i < size; ++i) {
    //     const RichTextSection section = sections[i];
    //     const char* str = section.text.data();
    //     const size_t length = section.text.size();
    //     const float scale = section.size / font.font_size;
    //     const float height = font.line_height * scale;

    //     const glm::vec3 color = section.color.to_vec3();

    //     uint32_t codepoint = 0;
    //     for (size_t i = 0; i < length;) {
    //         i += utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(str) + i, codepoint);

    //         if (codepoint == '\n') {
    //             y += height;
    //             x = position.x;
    //             continue;
    //         }

    //         auto it = font.glyphs.find(codepoint);
    //         if (it == font.glyphs.end()) {
    //             it = font.glyphs.find(0);
    //         }

    //         const sge::Glyph& ch = it->second;

    //         if (codepoint == ' ') {
    //             x += ch.advance * scale;
    //             continue;
    //         }

    //         const float xpos = x + ch.bearing.x * scale;
    //         const float ypos = y + height - ch.bearing.y * scale;
    //         const glm::vec2 pos = glm::vec2(xpos, ypos);
    //         const glm::vec2 size = glm::vec2(ch.size) * scale;

    //         const auto command = internal::DrawCommandGlyphSDF {
    //             .state = internal::BatchTextureState {
    //                 .texture = texture,
    //                 .scissor = scissor,
    //                 .order = order,
    //                 .blend_mode = m_blend_mode
    //             },
    //             .color = color,
    //             .pos = pos,
    //             .size = size,
    //             .tex_size = ch.data.sdf.tex_size,
    //             .tex_uv = ch.data.sdf.texture_coords,
    //         };

    //         m_glyph_sdf_draw_commands.push_back(command);

    //         ++m_glyph_sdf_data.total_count;

    //         x += ch.advance * scale;
    //     }
    // }

    return order;
}

uint32_t sge::Batch::AddSpriteDrawCommand(const BaseSprite& sprite, const glm::vec4& uv_offset_scale, const Texture& texture, struct Order custom_order) {
    ZoneScoped;

    const uint32_t order = GetOrder(custom_order);
    // const sge::IRect scissor = !m_scissors.empty() ? m_scissors.back() : sge::IRect();

    // const auto texture_with_sampler = internal::BatchTexture {
    //     .ptr = texture.internal().Get(),
    //     .sampler = texture.sampler()->internal().Get(),
    //     .id = texture.id()
    // };

    // const auto command = internal::DrawCommandSprite {
    //     .state = internal::BatchTextureState {
    //         .texture = texture_with_sampler,
    //         .scissor = scissor,
    //         .order = order,
    //         .blend_mode = m_blend_mode
    //     },
    //     .rotation = glm::quat(sprite.rotation()),
    //     .uv_offset_scale = uv_offset_scale,
    //     .color = sprite.color().to_vec4(),
    //     .outline_color = sprite.outline_color().to_vec4(),
    //     .position = glm::vec3(sprite.position(), sprite.z()),
    //     .size = sprite.size(),
    //     .offset = sprite.anchor().to_vec2(),
    //     .outline_thickness = sprite.outline_thickness(),
    // };

    // m_sprite_draw_commands.push_back(command);

    ++m_sprite_data.total_count;

    return order;
}

uint32_t sge::Batch::AddNinePatchDrawCommand(const NinePatch& ninepatch, const glm::vec4& uv_offset_scale, struct Order custom_order) {
    ZoneScoped;

    const uint32_t order = GetOrder(custom_order);
    // const sge::IRect scissor = !m_scissors.empty() ? m_scissors.back() : sge::IRect();

    // const auto texture = internal::BatchTexture {
    //     .ptr = ninepatch.texture().internal().Get(),
    //     .sampler = ninepatch.texture().sampler()->internal().Get(),
    //     .id = ninepatch.texture().id()
    // };

    // const auto command = internal::DrawCommandNinePatch {
    //     .state = internal::BatchTextureState {
    //         .texture = texture,
    //         .scissor = scissor,
    //         .order = order,
    //         .blend_mode = m_blend_mode
    //     },
    //     .rotation = glm::quat(ninepatch.rotation()),
    //     .uv_offset_scale = uv_offset_scale,
    //     .color = ninepatch.color().to_vec4(),
    //     .margin = ninepatch.margin(),
    //     .position = ninepatch.position(),
    //     .offset = ninepatch.anchor().to_vec2(),
    //     .source_size = glm::vec2(ninepatch.texture().size()),
    //     .output_size = ninepatch.size(),
    // };

    // m_ninepatch_draw_commands.push_back(command);

    ++m_ninepatch_data.total_count;

    return order;
}

uint32_t sge::Batch::DrawShape(Shape::Type shape, glm::vec2 position, glm::vec2 size, const sge::LinearRgba& color, const sge::LinearRgba& border_color, float border_thickness, BorderRadius border_radius, Anchor anchor, struct Order custom_order) {
    ZoneScoped;

    const float length = glm::min(size.x, size.y);

    const glm::vec4 radius = border_radius.is_relative()
        ? glm::vec4(border_radius.values()) / 100.0f * length
        : glm::vec4(border_radius.values());

    const uint32_t order = GetOrder(custom_order);
    // const sge::IRect scissor = !m_scissors.empty() ? m_scissors.back() : sge::IRect();

    // const auto command = internal::DrawCommandShape {
    //     .state = internal::BatchState {
    //         .scissor = scissor,
    //         .order = order,
    //         .blend_mode = m_blend_mode
    //     },
    //     .color = color,
    //     .border_color = border_color,
    //     .border_radius = radius,
    //     .position = glm::vec3(position, 0.0f),
    //     .size = size,
    //     .offset = anchor.to_vec2(),
    //     .border_thickness = border_thickness,
    //     .shape = shape,
    // };

    // m_shape_draw_commands.push_back(command);

    ++m_shape_data.total_count;

    return order;
}

uint32_t sge::Batch::DrawLine(glm::vec2 start, glm::vec2 end, float thickness, const sge::LinearRgba& color, BorderRadius border_radius, sge::Order custom_order) {
    ZoneScoped;

    glm::vec4 radius = glm::vec4(border_radius.values());
    if (border_radius.is_relative()) {
        const float length = glm::min(glm::length(glm::dot(start, end)), thickness);
        radius = glm::vec4(border_radius.values()) * length / 100.0f;
    }

    const uint32_t order = GetOrder(custom_order);
    // const sge::IRect scissor = !m_scissors.empty() ? m_scissors.back() : sge::IRect();

    // const auto command = internal::DrawCommandLine {
    //     .state = internal::BatchState {
    //         .scissor = scissor,
    //         .order = order,
    //         .blend_mode = m_blend_mode
    //     },
    //     .color = color,
    //     .border_radius = radius,
    //     .start = start,
    //     .end = end,
    //     .thickness = thickness,
    // };

    // m_line_draw_commands.push_back(command);

    ++m_line_data.total_count;

    return order;
}









sge::SpriteBatch::SpriteBatch(sge::Renderer& renderer, sge::Ref<LLGL::Shader> customPixelShader) {
    const auto& context = renderer.GetRenderContext();

    glm::vec2 vertices[] = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f)
    };

    LLGL::PipelineLayoutDescriptor layoutDesc;
    layoutDesc.bindings = sge::BindingLayout({
        sge::BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer", LLGL::StageFlags::VertexStage),
        sge::BindingLayoutItem::Texture(3, "Texture", LLGL::StageFlags::FragmentStage),
        sge::BindingLayoutItem::Sampler(4, "Sampler", LLGL::StageFlags::FragmentStage),
    });

    LLGL::VertexFormat vertexFormat = sge::VertexAttributes(context->Backend(), {
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "inp_position", "Position"),
    });

    LLGL::VertexFormat instanceFormat = sge::VertexAttributes(context->Backend(), vertexFormat.attributes.size(), {
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_rotation", "I_Rotation", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_uv_offset_scale", "I_UvOffsetScale", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_color", "I_Color", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_outline_color", "I_OutlineColor", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x3, "inp_i_position", "I_Position", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_offset", "I_Offset", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32, "inp_i_outline_thickness", "I_OutlineThickness", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags", 1),
    });

    std::vector<LLGL::VertexAttribute> totalAttributes = vertexFormat.attributes;
    totalAttributes.insert(totalAttributes.end(), instanceFormat.attributes.begin(), instanceFormat.attributes.end());

    ShaderSourceCode shader = GetSpriteShaderSourceCode(context->Backend());

    sge::Ref<LLGL::Shader> pixelShader = std::move(customPixelShader);
    if (!pixelShader.IsValid()) {
        pixelShader = context->CreateShader(sge::ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);
    }

    sge::GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.primitiveTopology = sge::PrimitiveTopology::TriangleStrip;
    pipelineConfig.layout = context->CreatePipelineLayout(layoutDesc);
    pipelineConfig.inputVertexAttribs = std::move(totalAttributes);
    pipelineConfig.vertexShader = context->CreateShader(sge::ShaderType::Vertex, "VS", shader.vs_source, shader.vs_size);
    pipelineConfig.pixelShader = pixelShader;
    pipelineConfig.blend.targets[0] = GetAlphaBlend();

    m_pipeline = context->CreateGraphicsPipeline(pipelineConfig);
    m_instance_buffer = sge::VertexBufferPool(std::move(instanceFormat));
    m_vertex_buffer = context->CreateVertexBuffer(vertices, vertexFormat);
}

void sge::SpriteBatch::Draw(const Sprite& sprite) {
    const auto& texture = sprite.texture();

    internal::BatchTexture texture_with_sampler;
    if (texture.is_valid()) {
        texture_with_sampler.ptr = texture.internal().Get();
        texture_with_sampler.sampler = texture.sampler()->internal().Get();
        texture_with_sampler.id = texture.id();
    }

    const auto state = internal::BatchState {
        .scissor = sge::IRect(),
        .resources_offset = 0,
        .resources_count = 0,
        .order = 0,
        .blend_mode = sge::BlendMode::AlphaBlend,
    };

    const glm::vec4 uv_offset_scale = internal::get_uv_offset_scale(sprite.flip_x(), sprite.flip_y());

    m_commands.push_back(DrawCommand {
        .instance_index = m_instances.size(),
        .state = state
    });
    m_instances.push_back(SpriteInstance {
        .rotation = glm::quat(sprite.rotation()),
        .uv_offset_scale = uv_offset_scale,
        .color = sprite.color().to_vec4(),
        .outline_color = sprite.outline_color().to_vec4(),
        .position = glm::vec3(sprite.position(), sprite.z()),
        .size = sprite.size(),
        .offset = sprite.anchor().to_vec2(),
        .outline_thickness = sprite.outline_thickness(),
        .flags = 0
    });
}


sge::LineBatch::LineBatch(sge::Renderer& renderer) {
    const auto& context = renderer.GetRenderContext();

    glm::vec2 vertices[] = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f)
    };

    LLGL::PipelineLayoutDescriptor layoutDesc;
    layoutDesc.bindings = sge::BindingLayout({
        sge::BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer", LLGL::StageFlags::VertexStage),
    });

    LLGL::VertexFormat vertexFormat = sge::VertexAttributes(context->Backend(), {
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "inp_position", "Position"),
    });

    LLGL::VertexFormat instanceFormat = sge::VertexAttributes(context->Backend(), vertexFormat.attributes.size(), {
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_start", "I_Start", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_end", "I_End", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_color", "I_Color", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_border_radius", "I_Border_Radius", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32, "inp_i_thickness", "I_Thickness", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags", 1),
    });

    std::vector<LLGL::VertexAttribute> totalAttributes = vertexFormat.attributes;
    totalAttributes.insert(totalAttributes.end(), instanceFormat.attributes.begin(), instanceFormat.attributes.end());

    ShaderSourceCode shader = GetLineShaderSourceCode(context->Backend());

    sge::GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.primitiveTopology = sge::PrimitiveTopology::TriangleStrip;
    pipelineConfig.layout = context->CreatePipelineLayout(layoutDesc);
    pipelineConfig.inputVertexAttribs = std::move(totalAttributes);
    pipelineConfig.vertexShader = context->CreateShader(sge::ShaderType::Vertex, "VS", shader.vs_source, shader.vs_size);
    pipelineConfig.pixelShader = context->CreateShader(sge::ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);
    pipelineConfig.blend.targets[0] = GetAlphaBlend();

    m_pipeline = context->CreateGraphicsPipeline(pipelineConfig);
    m_instance_buffer = sge::VertexBufferPool(std::move(instanceFormat));
    m_vertex_buffer = context->CreateVertexBuffer(vertices, vertexFormat);
}

void sge::LineBatch::Draw(glm::vec2 start, glm::vec2 end, float thickness, const sge::LinearRgba& color, BorderRadius border_radius, sge::Order custom_order) {
    ZoneScoped;

    glm::vec4 radius = glm::vec4(border_radius.values());
    if (border_radius.is_relative()) {
        const float length = glm::min(glm::length(glm::dot(start, end)), thickness);
        radius = glm::vec4(border_radius.values()) * length / 100.0f;
    }

    // const uint32_t order = GetOrder(custom_order);
    // const sge::IRect scissor = !m_scissors.empty() ? m_scissors.back() : sge::IRect();

    const auto state = internal::BatchState {
        .resources_offset = 0,
        .resources_count = 0,
    };

    m_commands.push_back(DrawCommand {
        .instance_index = m_instances.size(),
        .state = state
    });
    m_instances.push_back(LineInstance {
        .start = start,
        .end = end,
        .color = color.to_vec4(),
        .border_radius = radius,
        .thickness = thickness,
        .flags = 0
    });
}


sge::TextVectorBatch::TextVectorBatch(sge::Renderer& renderer) {
    const auto& context = renderer.GetRenderContext();

    glm::vec2 vertices[] = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f)
    };

    LLGL::PipelineLayoutDescriptor layoutDesc;
    layoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Buffer(3, "CurveBuffer", LLGL::StageFlags::FragmentStage),
        BindingLayoutItem::Buffer(4, "PartitionBuffer", LLGL::StageFlags::FragmentStage),
    });

    LLGL::VertexFormat vertexFormat = sge::VertexAttributes(context->Backend(), {
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "inp_position", "Position"),
    });

    LLGL::VertexFormat instanceFormat = sge::VertexAttributes(context->Backend(), vertexFormat.attributes.size(), {
        sge::Attribute::Instance(sge::VertexFormat::Float32x3, "inp_i_color", "I_Color", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_position", "I_Position", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_em_size", "I_Em_Size", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint32,    "inp_i_partition_offset", "I_PartitionOffset", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint32,    "inp_i_partition_count", "I_PartitionCount", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint8,     "inp_i_flags", "I_Flags", 1),
    });

    std::vector<LLGL::VertexAttribute> totalAttributes = vertexFormat.attributes;
    totalAttributes.insert(totalAttributes.end(), instanceFormat.attributes.begin(), instanceFormat.attributes.end());

    ShaderSourceCode shaderCode = GetFontVectorShaderSourceCode(context->Backend());

    sge::GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.debugName = "GlyphBatch Pipeline";
    pipelineConfig.vertexShader = context->CreateShader(sge::ShaderType::Vertex, "VS", shaderCode.vs_source, shaderCode.vs_size);
    pipelineConfig.pixelShader = context->CreateShader(sge::ShaderType::Fragment, "PS", shaderCode.fs_source, shaderCode.fs_size);
    pipelineConfig.inputVertexAttribs = std::move(totalAttributes);
    pipelineConfig.layout = context->CreatePipelineLayout(layoutDesc);
    pipelineConfig.primitiveTopology = sge::PrimitiveTopology::TriangleStrip;
    pipelineConfig.blend.targets[0] = GetAlphaBlend();

    m_pipeline = context->CreateGraphicsPipeline(pipelineConfig);
    m_instance_buffer = sge::VertexBufferPool(std::move(instanceFormat));
    m_vertex_buffer = context->CreateVertexBuffer(vertices, vertexFormat);
}

void sge::TextVectorBatch::Draw(const sge::RichTextSection* sections, size_t size, glm::vec2 position, const sge::FontVector& font, bool ui, sge::Order order) {
    ZoneScoped;

    float x = position.x;
    float y = position.y;

    const auto resources_offset = m_resources.size();
    m_resources.push_back(font.curve_buffer.Get());
    m_resources.push_back(font.partition_buffer.Get());
    const auto resources_count = m_resources.size() - resources_offset;

    uint8_t flags = 0;
    flags |= ui << SpriteFlags::UI;

    // const sge::IRect scissor = !m_scissors.empty() ? m_scissors.back() : sge::IRect();

    for (size_t i = 0; i < size; ++i) {
        const RichTextSection section = sections[i];
        const char* str = section.text.data();
        const size_t length = section.text.size();
        const float scale = section.size / font.units_per_em;
        const float height = (font.ascender - font.descender) * scale;

        const glm::vec3 color = section.color.to_vec3();

        uint32_t codepoint = 0;
        for (size_t i = 0; i < length;) {
            i += utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(str) + i, codepoint);

            if (codepoint == '\n') {
                y += height;
                x = position.x;
                continue;
            }

            auto it = font.glyphs.find(codepoint);
            if (it == font.glyphs.end()) {
                it = font.glyphs.find(0);
            }

            const sge::Glyph& ch = it->second;

            if (codepoint == ' ') {
                x += ch.advance * scale;
                continue;
            }

            const float xpos = x + ch.bearing.x * scale;
            const float ypos = y + height - ch.bearing.y * scale;
            const glm::vec2 pos = glm::vec2(xpos, ypos);
            const glm::vec2 size = glm::vec2(ch.size) * scale;

            const auto state = internal::BatchState {
                .resources_offset = resources_offset,
                .resources_count = resources_count
            };

            m_commands.push_back(DrawCommand {
                .instance_index = m_instances.size(),
                .state = state,
            });
            m_instances.push_back(GlyphInstanceVector {
                .color = color,
                .pos = pos,
                .size = size,
                .em_size = ch.size,
                .partition_offset = ch.data.vector.partition_offset,
                .partition_count = ch.data.vector.partition_count,
                .flags = flags
            });

            x += ch.advance * scale;
        }
    }
}