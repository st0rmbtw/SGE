#include <SGE/math/rect.hpp>
#include <SGE/profile.hpp>
#include <SGE/renderer/attributes.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/buffer_pool.hpp>
#include <SGE/renderer/material.hpp>
#include <SGE/renderer/mesh.hpp>
#include <SGE/renderer/renderer2d.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/utils.hpp>
#include <SGE/renderer/vertex_format.hpp>
#include <SGE/types/binding_layout.hpp>
#include <SGE/utils/utf8.hpp>

#include <LLGL/ShaderFlags.h>

#include <optional>

#include "shaders.hpp"

static constexpr float ComputeTextBaseline(sge::TextAlignment alignment, float y, float scale, int16_t ascender, int16_t descender) {
    switch (alignment) {
    case sge::TextAlignment::Top: {
        return y + float(ascender) * scale;
    }
    case sge::TextAlignment::Center: {
        const int16_t font_height_design = ascender - descender;
        return y + ((font_height_design * 0.5f) - descender) * scale;
    }
    case sge::TextAlignment::Bottom: {
        const int16_t font_height_design = ascender - descender;
        return y + font_height_design * scale;
    }
    }
}

sge::SpriteBatch::SpriteBatch(sge::Renderer& renderer, SpriteBatchDesc desc) {
    const auto& context = renderer.GetRenderContext();

    m_mesh = renderer.GetDefaultBatch2dMesh();

    ShaderSourceCode shaderCode = GetSpriteShaderSourceCode(context->Backend());

    sge::Ref<LLGL::Shader> pixelShader = std::move(desc.customPixelShader);
    if (!pixelShader.IsValid()) {
        pixelShader = context->CreateShader(sge::ShaderType::Fragment, "PS", shaderCode.fs_source, shaderCode.fs_size);
    }

    auto materialDesc = sge::MaterialDesc()
        .SetVertexShader(context->CreateShader(sge::ShaderType::Vertex, "VS", shaderCode.vs_source, shaderCode.vs_size))
        .SetFragmentShader(std::move(pixelShader))
        .SetScissorTestEnabled(desc.enableScissor)
        .SetDepthTestEnabled(desc.enableDepth)
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_rotation", "I_Rotation")
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_uv_offset_scale", "I_UvOffsetScale")
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_color", "I_Color")
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_outline_color", "I_OutlineColor")
        .AddInstanceAttribute(sge::VertexFormat::Float32x3, "inp_i_position", "I_Position")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_offset", "I_Offset")
        .AddInstanceAttribute(sge::VertexFormat::Float32, "inp_i_outline_thickness", "I_OutlineThickness")
        .AddInstanceAttribute(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags")
        .BindConstantBuffer(2, "GlobalUniformBuffer_std140", renderer.GlobalUniformBuffer(), LLGL::StageFlags::VertexStage)
        .AddDynamicTexture(3, "Texture", LLGL::StageFlags::FragmentStage)
        .AddDynamicSampler(4, "Sampler", LLGL::StageFlags::FragmentStage);

    for (uint8_t i = 0; i < 4; ++i) {
        m_materials[i] = renderer.CreateMaterial(
            materialDesc.SetBlendMode(static_cast<sge::BlendMode>(i))
        );
    }
}

uint32_t sge::SpriteBatch::AddSpriteDrawCommand(const sge::BaseSprite& sprite, const glm::vec4& uv_offset_scale, const sge::Texture& texture, std::optional<FlagsType> override_flags, sge::Order custom_order) {
    const auto bindings_offset = m_dynamic_bindings.size();
    if (texture.is_valid()) {
        m_dynamic_bindings.push_back(texture.internal().Get());
        m_dynamic_bindings.push_back(texture.sampler()->internal().Get());
    }
    const auto bindings_count = m_dynamic_bindings.size() - bindings_offset;

    auto& batchGroup = GetBatchGroup();

    const auto order = batchGroup.GetOrder(custom_order);
    const auto flags = override_flags.value_or(GetFlags());
    const auto scissor = batchGroup.GetCurrentScissor().value_or(sge::IRect());
    const auto blendMode = batchGroup.GetBlendMode();

    const auto state = internal::BatchState {
        .scissor = scissor,
        .resources_offset = bindings_offset,
        .resources_count = bindings_count,
        .order = order,
        .blend_mode = blendMode
    };

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
        .flags = flags.data()
    });

    return order;
}

uint32_t sge::SpriteBatch::DrawAtlasSprite(const sge::TextureAtlasSprite& sprite, sge::Order customOrder, std::optional<FlagsType> overrideFlags) {
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

    return AddSpriteDrawCommand(sprite, uv_offset_scale, sprite.atlas().texture(), overrideFlags, customOrder);
}

sge::NinePatchBatch::NinePatchBatch(sge::Renderer& renderer, NinePatchBatchDesc desc) {
    const auto& context = renderer.GetRenderContext();

    m_mesh = renderer.GetDefaultBatch2dMesh();

    ShaderSourceCode shaderCode = GetNinepatchShaderSourceCode(context->Backend());

    sge::Ref<LLGL::Shader> pixelShader = std::move(desc.customPixelShader);
    if (!pixelShader.IsValid()) {
        pixelShader = context->CreateShader(sge::ShaderType::Fragment, "PS", shaderCode.fs_source, shaderCode.fs_size);
    }

    auto materialDesc = sge::MaterialDesc()
        .SetVertexShader(context->CreateShader(sge::ShaderType::Vertex, "VS", shaderCode.vs_source, shaderCode.vs_size))
        .SetFragmentShader(std::move(pixelShader))
        .SetBlendMode(sge::BlendMode::AlphaBlend)
        .SetScissorTestEnabled(desc.enableScissor)
        .SetDepthTestEnabled(desc.enableDepth)
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_rotation", "I_Rotation")
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_color", "I_Color")
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_uv_offset_scale", "I_UvOffsetScale")
        .AddInstanceAttribute(sge::VertexFormat::Uint32x4,  "inp_i_margin", "I_Margin")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_position", "I_Position")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_offset", "I_Offset")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_source_size", "I_SourceSize")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_output_size", "I_OutputSize")
        .AddInstanceAttribute(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags")
        .BindConstantBuffer(2, "GlobalUniformBuffer_std140", renderer.GlobalUniformBuffer(), LLGL::StageFlags::VertexStage)
        .AddDynamicTexture(3, "Texture", LLGL::StageFlags::FragmentStage)
        .AddDynamicSampler(4, "Sampler", LLGL::StageFlags::FragmentStage);

    for (uint8_t i = 0; i < 4; ++i) {
        m_materials[i] = renderer.CreateMaterial(
            materialDesc.SetBlendMode(static_cast<sge::BlendMode>(i))
        );
    }
}

uint32_t sge::NinePatchBatch::Draw(const NinePatch& ninepatch, sge::Order customOrder, std::optional<FlagsType> overrideFlags) {
    ZoneScoped;

    const auto& texture = ninepatch.texture();

    const auto bindings_offset = m_dynamic_bindings.size();
    if (texture.is_valid()) {
        m_dynamic_bindings.push_back(texture.internal().Get());
        m_dynamic_bindings.push_back(texture.sampler()->internal().Get());
    }
    const auto bindings_count = m_dynamic_bindings.size() - bindings_offset;

    auto& batchGroup = GetBatchGroup();

    const auto order = batchGroup.GetOrder(customOrder);
    const auto flags = overrideFlags.value_or(GetFlags());
    const auto scissor = batchGroup.GetCurrentScissor().value_or(sge::IRect());
    const auto blendMode = batchGroup.GetBlendMode();

    const auto state = internal::BatchState {
        .scissor = scissor,
        .resources_offset = bindings_offset,
        .resources_count = bindings_count,
        .order = order,
        .blend_mode = blendMode
    };

    const glm::vec4 uv_offset_scale = internal::get_uv_offset_scale(ninepatch.flip_x(), ninepatch.flip_y());

    m_commands.push_back(DrawCommand {
        .instance_index = m_instances.size(),
        .state = state
    });
    m_instances.push_back(NinePatchInstance {
        .rotation = glm::quat(ninepatch.rotation()),
        .color = ninepatch.color().to_vec4(),
        .uv_offset_scale = uv_offset_scale,
        .margin = ninepatch.margin(),
        .position = ninepatch.position(),
        .offset = ninepatch.anchor().to_vec2(),
        .source_size = glm::vec2(ninepatch.texture().size()),
        .output_size = ninepatch.size(),
        .flags = flags.data()
    });

    return order;
}

sge::LineBatch::LineBatch(sge::Renderer& renderer, LineBatchDesc desc) {
    const auto& context = renderer.GetRenderContext();

    m_mesh = renderer.GetDefaultBatch2dMesh();

    ShaderSourceCode shaderCode = GetLineShaderSourceCode(context->Backend());

    auto materialDesc = sge::MaterialDesc()
        .SetVertexShader(context->CreateShader(sge::ShaderType::Vertex, "VS", shaderCode.vs_source, shaderCode.vs_size))
        .SetFragmentShader(context->CreateShader(sge::ShaderType::Fragment, "PS", shaderCode.fs_source, shaderCode.fs_size))
        .SetBlendMode(sge::BlendMode::AlphaBlend)
        .SetScissorTestEnabled(desc.enableScissor)
        .SetDepthTestEnabled(desc.enableDepth)
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_start", "I_Start")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_end", "I_End")
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_color", "I_Color")
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_border_radius", "I_Border_Radius")
        .AddInstanceAttribute(sge::VertexFormat::Float32, "inp_i_thickness", "I_Thickness")
        .AddInstanceAttribute(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags")
        .BindConstantBuffer(2, "GlobalUniformBuffer_std140", renderer.GlobalUniformBuffer(), LLGL::StageFlags::VertexStage);

    for (uint8_t i = 0; i < 4; ++i) {
        m_materials[i] = renderer.CreateMaterial(
            materialDesc.SetBlendMode(static_cast<sge::BlendMode>(i))
        );
    }
}

uint32_t sge::LineBatch::Draw(glm::vec2 start, glm::vec2 end, float thickness, const sge::LinearRgba& color, BorderRadius borderRadius, sge::Order customOrder, std::optional<FlagsType> overrideFlags) {
    ZoneScoped;

    glm::vec4 radius = glm::vec4(borderRadius.values());
    if (borderRadius.is_relative()) {
        const float length = glm::min(glm::length(glm::dot(start, end)), thickness);
        radius = glm::vec4(borderRadius.values()) * length / 100.0f;
    }

    auto& batchGroup = GetBatchGroup();

    const uint32_t order = batchGroup.GetOrder(customOrder);
    const auto flags = overrideFlags.value_or(GetFlags());
    const auto scissor = batchGroup.GetCurrentScissor().value_or(sge::IRect());
    const auto blendMode = batchGroup.GetBlendMode();

    const auto state = internal::BatchState {
        .scissor = scissor,
        .resources_offset = 0,
        .resources_count = 0,
        .order = order,
        .blend_mode = blendMode
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
        .flags = flags.data()
    });

    return order;
}

sge::TextSdfBatch::TextSdfBatch(sge::Renderer& renderer, TextSdfBatchDesc desc) {
    const auto& context = renderer.GetRenderContext();

    m_mesh = renderer.GetDefaultBatch2dMesh();

    ShaderSourceCode shaderCode = GetFontSdfShaderSourceCode(context->Backend());

    sge::Ref<LLGL::Shader> pixelShader = std::move(desc.customPixelShader);
    if (!pixelShader.IsValid()) {
        pixelShader = context->CreateShader(sge::ShaderType::Fragment, "PS", shaderCode.fs_source, shaderCode.fs_size);
    }

    auto materialDesc = sge::MaterialDesc()
        .SetVertexShader(context->CreateShader(sge::ShaderType::Vertex, "VS", shaderCode.vs_source, shaderCode.vs_size))
        .SetFragmentShader(std::move(pixelShader))
        .SetBlendMode(sge::BlendMode::AlphaBlend)
        .SetScissorTestEnabled(desc.enableScissor)
        .SetDepthTestEnabled(desc.enableDepth)
        .AddInstanceAttribute(sge::VertexFormat::Float32x3, "inp_i_color", "I_Color")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_position", "I_Position")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_tex_size", "I_TexSize")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_uv", "I_UV")
        .AddInstanceAttribute(sge::VertexFormat::Uint8,     "inp_i_flags", "I_Flags")
        .BindConstantBuffer(2, "GlobalUniformBuffer_std140", renderer.GlobalUniformBuffer(), LLGL::StageFlags::VertexStage)
        .AddDynamicTexture(3, "Texture", LLGL::StageFlags::FragmentStage)
        .AddDynamicSampler(4, "Sampler", LLGL::StageFlags::FragmentStage);

    for (uint8_t i = 0; i < 4; ++i) {
        m_materials[i] = renderer.CreateMaterial(
            materialDesc.SetBlendMode(static_cast<sge::BlendMode>(i))
        );
    }
}

uint32_t sge::TextSdfBatch::Draw(const sge::RichTextSection* sections, size_t size, glm::vec2 position, sge::TextAlignment alignment, const sge::Font& font, sge::Order customOrder, std::optional<FlagsType> overrideFlags) {
    ZoneScoped;

    const auto bindings_offset = m_dynamic_bindings.size();
    m_dynamic_bindings.push_back(font.texture.internal().Get());
    m_dynamic_bindings.push_back(font.texture.sampler()->internal().Get());
    const auto bindings_count = m_dynamic_bindings.size() - bindings_offset;

    auto& batchGroup = GetBatchGroup();

    const auto order = batchGroup.GetOrder(customOrder);
    const auto flags = overrideFlags.value_or(GetFlags());
    const auto scissor = batchGroup.GetCurrentScissor().value_or(sge::IRect());
    const auto blendMode = batchGroup.GetBlendMode();

    float x = position.x;
    float y = position.y;

    for (size_t sectionIdx = 0; sectionIdx < size; ++sectionIdx) {
        const RichTextSection section = sections[sectionIdx];
        const char* str = section.text.data();
        const size_t length = section.text.size();
        const float scale = (section.size / font.font_size);

        const glm::vec3 color = section.color.to_vec3();

        float baseline_y = ComputeTextBaseline(alignment, y, font.base_scale * scale, font.ascender, font.descender);

        uint32_t codepoint = 0;
        for (size_t i = 0; i < length;) {
            i += utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(str) + i, codepoint);

            if (codepoint == '\n') {
                const float height = (font.ascender - font.descender) * scale * font.base_scale;
                y += height;
                baseline_y = ComputeTextBaseline(alignment, y, font.base_scale * scale, font.ascender, font.descender);
                x = position.x;
                continue;
            }

            auto it = font.glyphs.find(codepoint);
            if (it == font.glyphs.end()) {
                it = font.glyphs.find(0);
            }

            const sge::Glyph& ch = it->second;

            if (codepoint == ' ') {
                x += ch.advance * scale * font.base_scale;
                continue;
            }

            const float xpos = x + ch.bearing.x * scale;
            const float ypos = baseline_y - ch.bearing.y * scale;

            const glm::vec2 pos = glm::vec2(xpos, ypos);
            const glm::vec2 size = glm::vec2(ch.size) * scale;

            m_commands.push_back(DrawCommand {
                .instance_index = m_instances.size(),
                .state = internal::BatchState {
                    .scissor = scissor,
                    .resources_offset = bindings_offset,
                    .resources_count = bindings_count,
                    .order = order,
                    .blend_mode = blendMode
                },
            });
            m_instances.push_back(GlyphInstanceSDF {
                .color = color,
                .pos = pos,
                .size = size,
                .tex_size = ch.data.sdf.tex_size,
                .uv = ch.data.sdf.texture_coords,
                .flags = flags.data()
            });

            x += ch.advance * scale * font.base_scale;
        }
    }

    return order;
}

sge::TextVectorBatch::TextVectorBatch(sge::Renderer& renderer, TextVectorBatchDesc desc) {
    const auto& context = renderer.GetRenderContext();

    m_mesh = renderer.GetDefaultBatch2dMesh();

    ShaderSourceCode shaderCode = GetFontVectorShaderSourceCode(context->Backend());

    auto materialDesc = sge::MaterialDesc()
        .SetVertexShader(context->CreateShader(sge::ShaderType::Vertex, "VS", shaderCode.vs_source, shaderCode.vs_size))
        .SetFragmentShader(context->CreateShader(sge::ShaderType::Fragment, "PS", shaderCode.fs_source, shaderCode.fs_size))
        .SetBlendMode(sge::BlendMode::AlphaBlend)
        .SetScissorTestEnabled(desc.enableScissor)
        .SetDepthTestEnabled(desc.enableDepth)
        .AddInstanceAttribute(sge::VertexFormat::Float32x3, "inp_i_color", "I_Color")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_position", "I_Position")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_em_size", "I_Em_Size")
        .AddInstanceAttribute(sge::VertexFormat::Uint32,    "inp_i_partition_offset", "I_PartitionOffset")
        .AddInstanceAttribute(sge::VertexFormat::Uint32,    "inp_i_partition_count", "I_PartitionCount")
        .AddInstanceAttribute(sge::VertexFormat::Uint8,     "inp_i_flags", "I_Flags")
        .BindConstantBuffer(2, "GlobalUniformBuffer_std140", renderer.GlobalUniformBuffer(), LLGL::StageFlags::VertexStage)
        .AddDynamicBuffer(3, "CurveBuffer", LLGL::StageFlags::FragmentStage)
        .AddDynamicBuffer(4, "PartitionBuffer", LLGL::StageFlags::FragmentStage);

    for (uint8_t i = 0; i < 4; ++i) {
        m_materials[i] = renderer.CreateMaterial(
            materialDesc.SetBlendMode(static_cast<sge::BlendMode>(i))
        );
    }
}

uint32_t sge::TextVectorBatch::Draw(const sge::RichTextSection* sections, size_t size, glm::vec2 position, sge::TextAlignment alignment, const sge::FontVector& font, sge::Order customOrder, std::optional<FlagsType> overrideFlags) {
    ZoneScoped;

    float x = position.x;
    float y = position.y;

    const auto bindings_offset = m_dynamic_bindings.size();
    m_dynamic_bindings.push_back(font.curve_buffer.Get());
    m_dynamic_bindings.push_back(font.partition_buffer.Get());
    const auto bindings_count = m_dynamic_bindings.size() - bindings_offset;

    auto& batchGroup = GetBatchGroup();

    const auto order = batchGroup.GetOrder(customOrder);
    const auto flags = overrideFlags.value_or(GetFlags());
    const auto scissor = batchGroup.GetCurrentScissor().value_or(sge::IRect());
    const auto blendMode = batchGroup.GetBlendMode();

    for (size_t sectionIdx = 0; sectionIdx < size; ++sectionIdx) {
        const RichTextSection section = sections[sectionIdx];
        const char* str = section.text.data();
        const size_t length = section.text.size();
        const float scale = section.size / font.units_per_em;

        const glm::vec3 color = section.color.to_vec3();

        float baseline_y = ComputeTextBaseline(alignment, y, scale, font.ascender, font.descender);

        uint32_t codepoint = 0;
        for (size_t i = 0; i < length;) {
            i += utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(str) + i, codepoint);

            if (codepoint == '\n') {
                const float height = (font.ascender - font.descender) * scale;
                y += height;
                baseline_y = ComputeTextBaseline(alignment, y, scale, font.ascender, font.descender);
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
            const float ypos = baseline_y - ch.bearing.y * scale;
            const glm::vec2 pos = glm::vec2(xpos, ypos);
            const glm::vec2 size = glm::vec2(ch.size) * scale;

            const auto state = internal::BatchState {
                .scissor = scissor,
                .resources_offset = bindings_offset,
                .resources_count = bindings_count,
                .order = order,
                .blend_mode = blendMode
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
                .flags = flags.data()
            });

            x += ch.advance * scale;
        }
    }

    return order;
}

sge::ShapeBatch::ShapeBatch(sge::Renderer& renderer, ShapeBatchDesc desc) {
    const auto& context = renderer.GetRenderContext();

    m_mesh = renderer.GetDefaultBatch2dMesh();

    ShaderSourceCode shaderCode = GetShapeShaderSourceCode(context->Backend());

    auto materialDesc = sge::MaterialDesc()
        .SetVertexShader(context->CreateShader(sge::ShaderType::Vertex, "VS", shaderCode.vs_source, shaderCode.vs_size))
        .SetFragmentShader(context->CreateShader(sge::ShaderType::Fragment, "PS", shaderCode.fs_source, shaderCode.fs_size))
        .SetBlendMode(sge::BlendMode::AlphaBlend)
        .SetScissorTestEnabled(desc.enableScissor)
        .SetDepthTestEnabled(desc.enableDepth)
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_color", "I_Color")
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_border_color", "I_BorderColor")
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_border_radius", "I_BorderRadius")
        .AddInstanceAttribute(sge::VertexFormat::Float32x3, "inp_i_position", "I_Position")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_offset", "I_Offset")
        .AddInstanceAttribute(sge::VertexFormat::Float32, "inp_i_border_thickness", "I_BorderThickness")
        .AddInstanceAttribute(sge::VertexFormat::Uint8, "inp_i_shape", "I_Shape")
        .AddInstanceAttribute(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags")
        .BindConstantBuffer(2, "GlobalUniformBuffer_std140", renderer.GlobalUniformBuffer(), LLGL::StageFlags::VertexStage);

    for (uint8_t i = 0; i < 4; ++i) {
        m_materials[i] = renderer.CreateMaterial(
            materialDesc.SetBlendMode(static_cast<sge::BlendMode>(i))
        );
    }
}

uint32_t sge::ShapeBatch::Draw(sge::Shape::Type shape, glm::vec2 position, glm::vec2 size, const sge::LinearRgba& color, const sge::LinearRgba& borderColor, float borderThickness, BorderRadius borderRadius, sge::Anchor anchor, sge::Order customOrder, std::optional<FlagsType> overrideFlags) {
    ZoneScoped;

    auto& batchGroup = GetBatchGroup();

    const uint32_t order = batchGroup.GetOrder(customOrder);
    const auto flags = overrideFlags.value_or(GetFlags());
    const auto scissor = batchGroup.GetCurrentScissor().value_or(sge::IRect());
    const auto blendMode = batchGroup.GetBlendMode();

    const float length = glm::min(size.x, size.y);

    const glm::vec4 radius = borderRadius.is_relative()
        ? glm::vec4(borderRadius.values()) / 100.0f * length
        : glm::vec4(borderRadius.values());

    const auto state = internal::BatchState {
        .scissor = scissor,
        .resources_offset = 0,
        .resources_count = 0,
        .order = order,
        .blend_mode = blendMode
    };

    m_commands.push_back(DrawCommand {
        .instance_index = m_instances.size(),
        .state = state
    });
    m_instances.push_back(ShapeInstance {
        .color = color.to_vec4(),
        .border_color = borderColor.to_vec4(),
        .border_radius = radius,
        .position = glm::vec3(position, 0.0f),
        .size = size,
        .offset = anchor.to_vec2(),
        .border_thickness = borderThickness,
        .shape = shape,
        .flags = flags.data(),
    });

    return order;
}