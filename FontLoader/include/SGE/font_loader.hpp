#ifndef SGE_FONT_LOADER_HPP_
#define SGE_FONT_LOADER_HPP_

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include <glm/vec2.hpp>

namespace sge::font_loader {

struct GlyphInfoSdf {
    glm::vec2 tex_size;
    glm::vec2 texture_coords;
    glm::ivec2 size;
    glm::ivec2 bearing;
    uint32_t character;
    float advance;
};

struct FontDataSDF {
    std::vector<GlyphInfoSdf> glyphs;
    std::unique_ptr<uint8_t[]> atlas_data;
    uint64_t atlas_data_size;
    uint32_t atlas_width;
    uint32_t atlas_height;
    float font_size;
    float base_scale;
    int16_t ascender;
    int16_t descender;
};

struct GlyphInfoVector {
    glm::ivec2 size;
    glm::ivec2 bearing;
    uint32_t partition_offset;
    uint32_t partition_count;
    uint32_t character;
    float advance;
};

struct FontDataVector {
    std::vector<GlyphInfoVector> glyphs;
    std::unique_ptr<uint8_t[]> curves_data;
    size_t curves_data_size;
    size_t curves_data_stride;
    std::unique_ptr<uint8_t[]> partitions_data;
    size_t partitions_data_size;
    size_t partitions_data_stride;
    uint16_t units_per_em;
    int16_t ascender;
    int16_t descender;
};

struct SdfSettings {
    uint16_t font_size = 96;
};

FontDataSDF LoadSdfFontDataFromBytes(std::span<const uint8_t> buffer, const SdfSettings& settings = {});
FontDataSDF LoadSdfFontDataFromFile(const std::string& path, const SdfSettings& settings = {});

FontDataVector LoadVectorFontDataFromBytes(std::span<const uint8_t> buffer);
FontDataVector LoadVectorFontDataFromFile(const std::string& path);

} // namespace sge::font_loader

#endif // SGE_FONT_LOADER_HPP_