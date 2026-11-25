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
#include <SGE/types/color.hpp>
#include <SGE/types/vector_shape.hpp>
#include <SGE/utils/hash.hpp>
#include <SGE/utils/utf8.hpp>

#include <LLGL/ShaderFlags.h>
#include <glm/packing.hpp>

#include "shaders.hpp"

namespace {

constexpr float ComputeTextBaseline(sge::TextAlignment alignment, float y, float scale, int16_t ascender, int16_t descender) {
    switch (alignment) {
    case sge::TextAlignment::Top: {
        return y + static_cast<float>(ascender) * scale;
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

}

sge::SpriteBatch::SpriteBatch(sge::Renderer& renderer, SpriteBatchDesc desc) {
    const auto& context = renderer.GetRenderContext();

    m_mesh = renderer.GetDefaultBatch2dMesh();

    EmbeddedShaderSource shaderCode = GetSpriteShaderSourceCode(context->Backend());

    sge::Ref<LLGL::Shader> pixelShader = std::move(desc.customPixelShader);
    if (!pixelShader.IsValid()) {
        pixelShader = context->CreateShader(sge::ShaderType::Fragment, "PS", shaderCode.fragment);
    }

    auto materialDesc = sge::MaterialDesc()
        .SetVertexShader(context->CreateShader(sge::ShaderType::Vertex, "VS", shaderCode.vertex))
        .SetFragmentShader(std::move(pixelShader))
        .SetScissorTestEnabled(desc.enableScissor)
        .SetDepthTestEnabled(desc.enableDepth)
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_rotation", "I_Rotation")
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_uv_offset_scale", "I_UvOffsetScale")
        .AddInstanceAttribute(sge::VertexFormat::Float32x3, "inp_i_position", "I_Position")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_offset", "I_Offset")
        .AddInstanceAttribute(sge::VertexFormat::Float32, "inp_i_outline_thickness", "I_OutlineThickness")
        .AddInstanceAttribute(sge::VertexFormat::Uint32, "inp_i_color", "I_Color")
        .AddInstanceAttribute(sge::VertexFormat::Uint32, "inp_i_outline_color", "I_OutlineColor")
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

void sge::SpriteBatch::AddSpriteDrawCommand(const sge::BaseSprite& sprite, const glm::vec4& uv_offset_scale, const sge::Texture& texture, const DrawState& state) {
    internal::DrawCommandState commandState{};
    commandState.scissor     = state.scissor;
    commandState.blendMode   = state.blendMode;
    commandState.resourceCount = 2;
    commandState.resources[0] = texture.internal().Get();
    commandState.resources[1] = texture.sampler()->internal().Get();

    const uint64_t key = internal::MakeSortKey(state.order, m_states.Intern(commandState));

    if (!m_runs.empty() && m_runs.back().key == key) {
        m_runs.back().count += 1;
    } else {
        m_runs.push_back({ .key=key, .first=static_cast<uint32_t>(m_instances.size()), .count=1 });
    }

    m_instances.push_back(SpriteInstance {
        .rotation = glm::quat(sprite.rotation()),
        .uv_offset_scale = uv_offset_scale,
        .position = glm::vec3(sprite.position(), sprite.z()),
        .size = sprite.size(),
        .offset = sprite.anchor().to_vec2(),
        .outline_thickness = sprite.outline_thickness(),
        .color_packed = glm::packUnorm4x8(sprite.color().to_vec4()),
        .outline_color_packed = glm::packUnorm4x8(sprite.outline_color().to_vec4()),
        .flags = state.customFlags.value_or(GetFlags()).data()
    });
}

void sge::SpriteBatch::Add(const sge::TextureAtlasSprite& sprite, const DrawState& state) {
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

    AddSpriteDrawCommand(sprite, uv_offset_scale, sprite.atlas().texture(), state);
}

sge::NinePatchBatch::NinePatchBatch(sge::Renderer& renderer, NinePatchBatchDesc desc) {
    const auto& context = renderer.GetRenderContext();

    m_mesh = renderer.GetDefaultBatch2dMesh();

    EmbeddedShaderSource shaderCode = GetNinepatchShaderSourceCode(context->Backend());

    sge::Ref<LLGL::Shader> pixelShader = std::move(desc.customPixelShader);
    if (!pixelShader.IsValid()) {
        pixelShader = context->CreateShader(sge::ShaderType::Fragment, "PS", shaderCode.fragment);
    }

    auto materialDesc = sge::MaterialDesc()
        .SetVertexShader(context->CreateShader(sge::ShaderType::Vertex, "VS", shaderCode.vertex))
        .SetFragmentShader(std::move(pixelShader))
        .SetBlendMode(sge::BlendMode::AlphaBlend)
        .SetScissorTestEnabled(desc.enableScissor)
        .SetDepthTestEnabled(desc.enableDepth)
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_rotation", "I_Rotation")
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_uv_offset_scale", "I_UvOffsetScale")
        .AddInstanceAttribute(sge::VertexFormat::Uint32x4,  "inp_i_margin", "I_Margin")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_position", "I_Position")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_offset", "I_Offset")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_source_size", "I_SourceSize")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_output_size", "I_OutputSize")
        .AddInstanceAttribute(sge::VertexFormat::Uint32, "inp_i_color", "I_Color")
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

void sge::NinePatchBatch::Add(const NinePatch& ninepatch, const DrawState& state) {
    ZoneScoped;

    const auto& texture = ninepatch.texture();

    internal::DrawCommandState commandState{};
    commandState.scissor     = state.scissor;
    commandState.blendMode   = state.blendMode;
    commandState.resourceCount = 2;
    commandState.resources[0] = texture.internal().Get();
    commandState.resources[1] = texture.sampler()->internal().Get();

    const uint64_t key = internal::MakeSortKey(state.order, m_states.Intern(commandState));

    if (!m_runs.empty() && m_runs.back().key == key) {
        m_runs.back().count += 1;
    } else {
        m_runs.push_back({ .key=key, .first=static_cast<uint32_t>(m_instances.size()), .count=1 });
    }

    const glm::vec4 uv_offset_scale = internal::get_uv_offset_scale(ninepatch.flip_x(), ninepatch.flip_y());
    m_instances.push_back(NinePatchInstance {
        .rotation = glm::quat(ninepatch.rotation()),
        .uv_offset_scale = uv_offset_scale,
        .margin = ninepatch.margin(),
        .position = ninepatch.position(),
        .offset = ninepatch.anchor().to_vec2(),
        .source_size = glm::vec2(ninepatch.texture().size()),
        .output_size = ninepatch.size(),
        .color_packed = glm::packUnorm4x8(ninepatch.color().to_vec4()),
        .flags = state.customFlags.value_or(GetFlags()).data()
    });
}

sge::LineBatch::LineBatch(sge::Renderer& renderer, LineBatchDesc desc) {
    const auto& context = renderer.GetRenderContext();

    m_mesh = renderer.GetDefaultBatch2dMesh();

    EmbeddedShaderSource shaderCode = GetLineShaderSourceCode(context->Backend());

    auto materialDesc = sge::MaterialDesc()
        .SetVertexShader(context->CreateShader(sge::ShaderType::Vertex, "VS", shaderCode.vertex))
        .SetFragmentShader(context->CreateShader(sge::ShaderType::Fragment, "PS", shaderCode.fragment))
        .SetBlendMode(sge::BlendMode::AlphaBlend)
        .SetScissorTestEnabled(desc.enableScissor)
        .SetDepthTestEnabled(desc.enableDepth)
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_start", "I_Start")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_end", "I_End")
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_corner_radii", "I_CornerRadii")
        .AddInstanceAttribute(sge::VertexFormat::Float32,   "inp_i_thickness", "I_Thickness")
        .AddInstanceAttribute(sge::VertexFormat::Uint32,    "inp_i_color", "I_Color")
        .AddInstanceAttribute(sge::VertexFormat::Uint8,     "inp_i_flags", "I_Flags")
        .BindConstantBuffer(2, "GlobalUniformBuffer_std140", renderer.GlobalUniformBuffer(), LLGL::StageFlags::VertexStage);

    for (uint8_t i = 0; i < 4; ++i) {
        m_materials[i] = renderer.CreateMaterial(
            materialDesc.SetBlendMode(static_cast<sge::BlendMode>(i))
        );
    }
}

void sge::LineBatch::Add(const LineDrawData& data, const DrawState& state) {
    glm::vec4 radius = glm::vec4(data.borderRadius.values());
    if (data.borderRadius.is_relative()) {
        const float length = glm::min(glm::distance(data.start, data.end), data.thickness);
        radius = glm::vec4(data.borderRadius.values()) * length / 100.0f;
    }

    internal::DrawCommandState commandState{};
    commandState.scissor   = state.scissor;
    commandState.blendMode = state.blendMode;

    const uint64_t key = internal::MakeSortKey(state.order, m_states.Intern(commandState));

    if (!m_runs.empty() && m_runs.back().key == key) {
        m_runs.back().count += 1;
    } else {
        m_runs.push_back({ .key=key, .first=static_cast<uint32_t>(m_instances.size()), .count=1 });
    }

    auto& created = m_instances.emplace_back();
    created.start = data.start;
    created.end = data.end;
    created.border_radius = radius;
    created.thickness = data.thickness;
    created.color_packed = glm::packUnorm4x8(data.color.as_vec4());
    created.flags = state.customFlags.value_or(GetFlags()).data();
}

sge::TextSdfBatch::TextSdfBatch(sge::Renderer& renderer, TextSdfBatchDesc desc) {
    const auto& context = renderer.GetRenderContext();

    m_mesh = renderer.GetDefaultBatch2dMesh();

    EmbeddedShaderSource shaderCode = GetFontSdfShaderSourceCode(context->Backend());

    sge::Ref<LLGL::Shader> pixelShader = std::move(desc.customPixelShader);
    if (!pixelShader.IsValid()) {
        pixelShader = context->CreateShader(sge::ShaderType::Fragment, "PS", shaderCode.fragment);
    }

    auto materialDesc = sge::MaterialDesc()
        .SetVertexShader(context->CreateShader(sge::ShaderType::Vertex, "VS", shaderCode.vertex))
        .SetFragmentShader(std::move(pixelShader))
        .SetBlendMode(sge::BlendMode::AlphaBlend)
        .SetScissorTestEnabled(desc.enableScissor)
        .SetDepthTestEnabled(desc.enableDepth)
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_position", "I_Position")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_tex_size", "I_TexSize")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_uv", "I_UV")
        .AddInstanceAttribute(sge::VertexFormat::Uint32,    "inp_i_color", "I_Color")
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

void sge::TextSdfBatch::Add(const SdfTextDrawData& data, const DrawState& state) {
    ZoneScoped;

    float x = data.position.x;
    float y = data.position.y;

    size_t instancesStart = m_instances.size();

    for (size_t sectionIdx = 0; sectionIdx < data.sections.size(); ++sectionIdx) {
        const RichTextSection section = data.sections[sectionIdx];
        const char* str = section.text.data();
        const size_t length = section.text.size();
        const float scale = (section.size / data.font.font_size);

        const glm::vec3 color = section.color.to_vec3();

        float baseline_y = ComputeTextBaseline(data.alignment, y, data.font.base_scale * scale, data.font.ascender, data.font.descender);

        uint32_t codepoint = 0;
        for (size_t i = 0; i < length;) {
            i += utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(str) + i, codepoint);

            if (codepoint == '\n') {
                const float height = (data.font.ascender - data.font.descender) * scale * data.font.base_scale;
                y += height;
                baseline_y = ComputeTextBaseline(data.alignment, y, data.font.base_scale * scale, data.font.ascender, data.font.descender);
                x = data.position.x;
                continue;
            }

            auto it = data.font.glyphs.find(codepoint);
            if (it == data.font.glyphs.end()) {
                it = data.font.glyphs.find(0);
            }

            const sge::Glyph& ch = it->second;

            if (codepoint == ' ') {
                x += ch.advance * scale * data.font.base_scale;
                continue;
            }

            const float xpos = x + ch.bearing.x * scale;
            const float ypos = baseline_y - ch.bearing.y * scale;

            const glm::vec2 pos = glm::vec2(xpos, ypos);
            const glm::vec2 size = glm::vec2(ch.size) * scale;

            m_instances.push_back(GlyphInstanceSDF {
                .pos = pos,
                .size = size,
                .tex_size = ch.data.sdf.tex_size,
                .uv = ch.data.sdf.texture_coords,
                .color_packed = glm::packUnorm4x8(glm::vec4(color, 1.0)),
                .flags = state.customFlags.value_or(GetFlags()).data()
            });

            x += ch.advance * scale * data.font.base_scale;
        }
    }

    const uint32_t totalInstances = m_instances.size() - instancesStart;

    internal::DrawCommandState commandState{};
    commandState.scissor     = state.scissor;
    commandState.blendMode   = state.blendMode;
    commandState.resourceCount = 2;
    commandState.resources[0] = data.font.texture.internal().Get();
    commandState.resources[1] = data.font.texture.sampler()->internal().Get();

    const uint64_t key = internal::MakeSortKey(state.order, m_states.Intern(commandState));

    if (!m_runs.empty() && m_runs.back().key == key) {
        m_runs.back().count += totalInstances;
    } else {
        m_runs.push_back({ .key=key, .first=static_cast<uint32_t>(instancesStart), .count=totalInstances });
    }
}

void sge::TextVectorBatch::Add(const VectorTextDrawData& data, const DrawState& state) {
    ZoneScoped;

    float x = data.position.x;
    float y = data.position.y;

    size_t instancesStart = m_instances.size();

    for (size_t sectionIdx = 0; sectionIdx < data.sections.size(); ++sectionIdx) {
        const RichTextSection section = data.sections[sectionIdx];
        const char* str = section.text.data();
        const size_t length = section.text.size();
        const float sx = section.size / data.font.units_per_em;
        const float xhPx = std::max(1.0f, std::round(data.font.xHeight * sx));
        const float sy = xhPx / data.font.xHeight;

        const glm::vec3 color = section.color.to_vec3();

        float baseline = std::round(ComputeTextBaseline(data.alignment, y, sy, data.font.ascender, data.font.descender));

        uint32_t codepoint = 0;
        for (size_t i = 0; i < length;) {
            i += utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(str) + i, codepoint);

            if (codepoint == '\n') {
                y += (data.font.ascender - data.font.descender) * sy;
                baseline = std::round(ComputeTextBaseline(data.alignment, y, sy, data.font.ascender, data.font.descender));
                x = data.position.x;
                continue;
            }

            auto it = data.font.glyphs.find(codepoint);
            if (it == data.font.glyphs.end()) {
                it = data.font.glyphs.find(0);
            }

            const sge::Glyph& ch = it->second;

            if (codepoint == ' ') {
                x += ch.advance * sx;
                continue;
            }

            const float xpos = std::round(x) + ch.bearing.x * sx;
            const float ypos = baseline - ch.bearing.y * sy;
            const glm::vec2 pos = glm::vec2(xpos - 1.0f, ypos - 1.0f);
            const glm::vec2 size = glm::vec2(ch.size + 1) * glm::vec2(sx, sy);

            const auto& layer = data.font.vectorShape->GetLayer(ch.data.vector.layerId);

            m_instances.push_back(VectorShapeInstance {
                .pos = pos,
                .world_size = size,
                .shape_size = ch.size,
                .band_height = layer.bandHeight,
                .color_packed = glm::packUnorm4x8(glm::vec4(color, 1.0)),
                .partition_offset = layer.partitionOffset,
                .partition_count = layer.partitionCount,
                .flags = state.customFlags.value_or(GetFlags()).data()
            });

            x += ch.advance * sx;
        }
    }

    uint32_t totalInstances = m_instances.size() - instancesStart;

    internal::DrawCommandState commandState{};
    commandState.scissor     = state.scissor;
    commandState.blendMode   = state.blendMode;
    commandState.resourceCount = 2;
    commandState.resources[0] = &data.font.vectorShape->GetCurvesBuffer();
    commandState.resources[1] = &data.font.vectorShape->GetPartitionsBuffer();

    const uint64_t key = internal::MakeSortKey(state.order, m_states.Intern(commandState));

    if (!m_runs.empty() && m_runs.back().key == key) {
        m_runs.back().count += totalInstances;
    } else {
        m_runs.push_back({ .key=key, .first=static_cast<uint32_t>(instancesStart), .count=totalInstances });
    }
}

sge::VectorShapeBatch::VectorShapeBatch(sge::Renderer& renderer, TextVectorBatchDesc desc) {
    const auto& context = renderer.GetRenderContext();

    m_mesh = renderer.GetDefaultBatch2dMesh();

    EmbeddedShaderSource shaderCode = GetVectorShapeShaderSourceCode(context->Backend());

    auto materialDesc = sge::MaterialDesc()
        .SetVertexShader(context->CreateShader(sge::ShaderType::Vertex, "VS", shaderCode.vertex))
        .SetFragmentShader(context->CreateShader(sge::ShaderType::Fragment, "PS", shaderCode.fragment))
        .SetBlendMode(sge::BlendMode::AlphaBlend)
        .SetScissorTestEnabled(desc.enableScissor)
        .SetDepthTestEnabled(desc.enableDepth)
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_position", "I_Position")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "i_shape_size", "I_ShapeSize")
        .AddInstanceAttribute(sge::VertexFormat::Float32,   "i_band_height", "I_BandHeight")
        .AddInstanceAttribute(sge::VertexFormat::Uint32,    "inp_i_color", "I_Color")
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

void sge::VectorShapeBatch::Add(const VectorShapeDrawData& data, const DrawState& state) {
    ZoneScoped;

    internal::DrawCommandState commandState{};
    commandState.scissor     = state.scissor;
    commandState.blendMode   = state.blendMode;
    commandState.resourceCount = 2;
    commandState.resources[0] = &data.shape->GetCurvesBuffer();
    commandState.resources[1] = &data.shape->GetPartitionsBuffer();
    const uint64_t key = internal::MakeSortKey(state.order, m_states.Intern(commandState));

    const uint32_t instancesOffset = m_instances.size();

    if (data.layer == VectorShapeLayerId::ALL) {
        for (const auto& l : data.shape->GetLayers()) {
            m_instances.push_back(VectorShapeInstance {
                .pos = data.position,
                .world_size = data.size,
                .shape_size = l.size,
                .band_height = l.bandHeight,
                .color_packed = glm::packUnorm4x8(data.tint.as_vec4()),
                .partition_offset = l.partitionOffset,
                .partition_count = l.partitionCount,
                .flags = state.customFlags.value_or(GetFlags()).data()
            });
        }
    } else {
        const GpuVectorShapeLayer& l = data.shape->GetLayer(data.layer);
        m_instances.push_back(VectorShapeInstance {
            .pos = data.position,
            .world_size = data.size,
            .shape_size = l.size,
            .band_height = l.bandHeight,
            .color_packed = glm::packUnorm4x8(data.tint.as_vec4()),
            .partition_offset = l.partitionOffset,
            .partition_count = l.partitionCount,
            .flags = state.customFlags.value_or(GetFlags()).data()
        });
    }

    const uint32_t nInstances = m_instances.size() - instancesOffset;

    if (!m_runs.empty() && m_runs.back().key == key) {
        m_runs.back().count += nInstances;
    } else {
        m_runs.push_back({ .key=key, .first=instancesOffset, .count=nInstances });
    }
}

sge::ShapeBatch::ShapeBatch(sge::Renderer& renderer, ShapeBatchDesc desc) {
    const auto& context = renderer.GetRenderContext();

    m_mesh = renderer.GetDefaultBatch2dMesh();

    EmbeddedShaderSource shaderCode = GetShapeShaderSourceCode(context->Backend());

    auto materialDesc = sge::MaterialDesc()
        .SetVertexShader(context->CreateShader(sge::ShaderType::Vertex, "VS", shaderCode.vertex))
        .SetFragmentShader(context->CreateShader(sge::ShaderType::Fragment, "PS", shaderCode.fragment))
        .SetBlendMode(sge::BlendMode::AlphaBlend)
        .SetScissorTestEnabled(desc.enableScissor)
        .SetDepthTestEnabled(desc.enableDepth)
        .AddInstanceAttribute(sge::VertexFormat::Float32x4, "inp_i_corner_radii", "I_CornerRadii")
        .AddInstanceAttribute(sge::VertexFormat::Float32x3, "inp_i_position", "I_Position")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size")
        .AddInstanceAttribute(sge::VertexFormat::Float32x2, "inp_i_offset", "I_Offset")
        .AddInstanceAttribute(sge::VertexFormat::Float32,   "inp_i_border_thickness", "I_BorderThickness")
        .AddInstanceAttribute(sge::VertexFormat::Uint32,    "inp_i_color", "I_Color")
        .AddInstanceAttribute(sge::VertexFormat::Uint32,    "inp_i_border_color", "I_BorderColor")
        .AddInstanceAttribute(sge::VertexFormat::Uint8,     "inp_i_shape", "I_Shape")
        .AddInstanceAttribute(sge::VertexFormat::Uint8,     "inp_i_flags", "I_Flags")
        .BindConstantBuffer(2, "GlobalUniformBuffer_std140", renderer.GlobalUniformBuffer(), LLGL::StageFlags::VertexStage);

    for (uint8_t i = 0; i < 4; ++i) {
        m_materials[i] = renderer.CreateMaterial(
            materialDesc.SetBlendMode(static_cast<sge::BlendMode>(i))
        );
    }
}

void sge::ShapeBatch::Add(const sge::ShapeDrawData& data, const sge::DrawState& state) {
    ZoneScoped;

    const float length = glm::min(data.size.x, data.size.y);

    const glm::vec4 radius = data.borderRadius.is_relative()
        ? glm::vec4(data.borderRadius.values()) / 100.0f * length
        : glm::vec4(data.borderRadius.values());

    internal::DrawCommandState commandState{};
    commandState.scissor     = state.scissor;
    commandState.blendMode   = state.blendMode;
    commandState.resourceCount = 0;

    const uint64_t key = internal::MakeSortKey(state.order, m_states.Intern(commandState));

    if (!m_runs.empty() && m_runs.back().key == key) {
        m_runs.back().count += 1;
    } else {
        m_runs.push_back({ .key=key, .first=static_cast<uint32_t>(m_instances.size()), .count=1 });
    }

    m_instances.push_back(ShapeInstance {
        .border_radius = radius,
        .position = glm::vec3(data.position, 0.0f),
        .size = data.size,
        .offset = data.anchor.to_vec2(),
        .border_thickness = data.borderThickness,
        .color_packed = glm::packUnorm4x8(data.color.to_vec4()),
        .border_color_packed = glm::packUnorm4x8(data.borderColor.to_vec4()),
        .shape = data.type,
        .flags = state.customFlags.value_or(GetFlags()).data(),
    });
}

void sge::BatchGroup::Submit() {
    for (auto& batch : m_batches) {
        if (!batch) continue;
        m_renderer->SubmitBatch(batch);
    }
}
