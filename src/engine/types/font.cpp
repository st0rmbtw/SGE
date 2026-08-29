#include <vector>

#include <SGE/renderer/context.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/types/font.hpp>

#include <glm/common.hpp>
#include <glm/gtx/norm.hpp>

sge::FontVector sge::LoadVectorFontFromData(const sge::font_loader::FontDataVector& data, sge::RenderContext& context) {
    std::unordered_map<uint32_t, sge::Glyph> glyph_map;
    glyph_map.reserve(data.glyphs.size());

    for (const sge::font_loader::GlyphInfoVector& g : data.glyphs) {
        glyph_map[g.character].data.vector.partition_count = g.partition_count;
        glyph_map[g.character].data.vector.partition_offset = g.partition_offset;
        glyph_map[g.character].size = g.size;
        glyph_map[g.character].bearing = g.bearing;
        glyph_map[g.character].advance = g.advance;
    }

    return FontVector {
        .glyphs = std::move(glyph_map),
        .curve_buffer = context.CreateStructuredBuffer(data.curves_data_size, data.curves_data_stride, data.curves_data.get()),
        .partition_buffer = context.CreateStructuredBuffer(data.partitions_data_size, data.partitions_data_stride, data.partitions_data.get()),
        .units_per_em = data.units_per_em,
        .ascender = data.ascender,
        .descender = data.descender
    };
}

sge::FontVector sge::LoadVectorFontFromBytes(std::span<const uint8_t> buffer, sge::RenderContext& context) {
    auto data = sge::font_loader::LoadVectorFontDataFromBytes(buffer);
    return LoadVectorFontFromData(data, context);
}

sge::FontVector sge::LoadVectorFontFromFile(const std::string& path, sge::RenderContext& context) {
    auto data = sge::font_loader::LoadVectorFontDataFromFile(path);
    return LoadVectorFontFromData(data, context);
}

sge::Font sge::LoadFontFromData(const sge::font_loader::FontDataSDF& data, class sge::RenderContext& context) {
    sge::TextureConfig textureConfig;
    textureConfig.debugName = "Default Font Texture";
    textureConfig.textureType = LLGL::TextureType::Texture2D;
    textureConfig.extent.width = data.atlas_width;
    textureConfig.extent.height = data.atlas_height;
    textureConfig.extent.depth = 1;
    textureConfig.sampler = context.GetLinearSampler();

    LLGL::ImageView imageView;
    imageView.dataType = LLGL::DataType::UInt8;
    imageView.format = LLGL::ImageFormat::R;
    imageView.dataSize = textureConfig.extent.width * textureConfig.extent.height;
    imageView.data = data.atlas_data.get();

    std::unordered_map<uint32_t, sge::Glyph> glyph_map;
    glyph_map.reserve(data.glyphs.size());

    for (const sge::font_loader::GlyphInfoSdf& g : data.glyphs) {
        glyph_map[g.character].data.sdf.tex_size = g.tex_size;
        glyph_map[g.character].data.sdf.texture_coords = g.texture_coords;
        glyph_map[g.character].size = g.size;
        glyph_map[g.character].bearing = g.bearing;
        glyph_map[g.character].advance = g.advance;
    }

    return sge::Font {
        .glyphs = glyph_map,
        .texture = context.CreateTexture(textureConfig, &imageView),
        .font_size = data.font_size,
        .base_scale = data.base_scale,
        .ascender = data.ascender,
        .descender = data.descender
    };
}

sge::Font sge::LoadFontFromBytes(std::span<const uint8_t> buffer, class sge::RenderContext& context) {
    auto data = sge::font_loader::LoadSdfFontDataFromBytes(buffer);
    return LoadFontFromData(data, context);
}

sge::Font sge::LoadFontFromFile(const std::string& path, sge::RenderContext& context) {
    auto data = sge::font_loader::LoadSdfFontDataFromFile(path);
    return LoadFontFromData(data, context);
}