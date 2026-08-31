#include <print>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
#include FT_OUTLINE_H
#include FT_ADVANCES_H

#include <SGE/font_loader.hpp>

#include "rectpack2D/finders_interface.h"

#include "common.hpp"

namespace {
    constexpr uint32_t SDF_PADDING = 2;

    struct GlyphInfo {
        std::unique_ptr<uint8_t[]> buffer;
        FT_ULong character;
        uint32_t width;
        uint32_t height;
        FT_Int bitmap_left;
        FT_Int bitmap_top;
        float advance_x;
        uint32_t col;
        uint32_t row;
    };

    using spaces_type = rectpack2D::empty_spaces<false>;
    using rect_type = rectpack2D::output_rect_t<spaces_type>;

    struct rect {
        rect_type xywh;
        size_t index = 0;

        rect_type& get_rect() {
			return xywh;
		}

		[[nodiscard]]
        const rect_type& get_rect() const {
			return xywh;
		}
    };
} // namespace

sge::font_loader::FontDataSDF sge::font_loader::LoadSdfFontDataFromBytes(
    std::span<const uint8_t> buffer,
    const SdfSettings& settings
) {
    FT_Library library;
    FT_Init_FreeType(&library);

    FT_UInt overlapping = 1; // Enables better handling of complex intersecting paths
    FT_Property_Set(library, "sdf", "overlapping", &overlapping);

    FT_Face face;
    FT_New_Memory_Face(library, buffer.data(), buffer.size(), 0, &face);
    FT_Set_Pixel_Sizes(face, 0, settings.font_size);

    FT_UInt index;
    FT_ULong character = FT_Get_First_Char(face, &index);

    std::vector<GlyphInfo> glyph_infos;

    while (true) {
        FT_Load_Char(face, character, FT_LOAD_DEFAULT);
        FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF);

        FT_Bitmap* bitmap = &face->glyph->bitmap;

        std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(static_cast<size_t>(bitmap->rows) * bitmap->width);
        for (uint32_t y = 0; y < bitmap->rows; ++y) {
            uint8_t* row_data = bitmap->buffer + y * bitmap->pitch;
            for (uint32_t x = 0; x < bitmap->width; ++x) {
                const size_t idx = y * bitmap->width + x;
                buffer[idx] = row_data[x];
            }
        }

        FT_Fixed unscaled_advance;
        FT_Get_Advance(face, index, FT_LOAD_NO_SCALE, &unscaled_advance);

        glyph_infos.push_back(GlyphInfo{
            .buffer = std::move(buffer),
            .character = character,
            .width = bitmap->width,
            .height = bitmap->rows,
            .bitmap_left = face->glyph->bitmap_left,
            .bitmap_top = face->glyph->bitmap_top,
            .advance_x = float(unscaled_advance),
            .col = 0,
            .row = 0
        });

        if (!index) break;

        character = FT_Get_Next_Char(face, character, &index);
    }

    std::vector<rect> rectangles;
    for (size_t i = 0; i < glyph_infos.size(); ++i) {
        const GlyphInfo& g = glyph_infos[i];
        rectangles.emplace_back(rectpack2D::rect_xywh(0, 0, g.width + SDF_PADDING*2, g.height + SDF_PADDING*2), i);
    }

    const auto result_size = find_best_packing<spaces_type>(
        rectangles,
        make_finder_input(
            rectpack2D::rect_wh(32768, 32768),
            -4,
            [](rect_type&) { return rectpack2D::callback_result::CONTINUE_PACKING; },
            [](rect_type&) {
                std::println(stderr, "[ERROR] Failed to pack glyphs");
                std::abort();
                return rectpack2D::callback_result::ABORT_PACKING;
            },
            rectpack2D::flipping_option::DISABLED
        )
    );

    for (auto& rect : rectangles) {
        glyph_infos[rect.index].col = rect.xywh.x + SDF_PADDING;
        glyph_infos[rect.index].row = rect.xywh.y + SDF_PADDING;
    }

    const uint32_t atlas_width = result_size.w;
    const uint32_t atlas_height = result_size.h;

    const size_t atlas_data_size = atlas_width * atlas_height;
    auto atlas_data = std::make_unique<uint8_t[]>(atlas_data_size);
    memset(atlas_data.get(), 0x0, atlas_data_size * sizeof(atlas_data[0]));

    for (const auto& g : glyph_infos) {
        for (uint32_t y = 0; y < g.height; ++y) {
            size_t dst_idx = (g.row + y) * atlas_width + g.col;
            size_t src_idx = y * g.width;
            memcpy(&atlas_data[dst_idx], &g.buffer[src_idx], g.width);
        }
    }

    std::vector<GlyphInfoSdf> glyphs;
    glyphs.reserve(glyph_infos.size());

    for (const auto& g : glyph_infos) {
        float tex_width = float(g.width) / atlas_width;
        float tex_height = float(g.height) / atlas_height;
        float texture_coord_x = float(g.col) / atlas_width;
        float texture_coord_y = float(g.row) / atlas_height;
        glyphs.push_back(GlyphInfoSdf {
            .tex_size = glm::vec2(tex_width, tex_height),
            .texture_coords = glm::vec2(texture_coord_x, texture_coord_y),
            .size = glm::ivec2(g.width, g.height),
            .bearing = glm::ivec2(g.bitmap_left, g.bitmap_top),
            .character = g.character,
            .advance = g.advance_x
        });
    }

    const float base_scale = static_cast<float>(settings.font_size) / static_cast<float>(face->units_per_EM);

    return FontDataSDF {
        .glyphs = std::move(glyphs),
        .atlas_data = std::move(atlas_data),
        .atlas_data_size = atlas_data_size,
        .atlas_width = atlas_width,
        .atlas_height = atlas_height,
        .font_size = static_cast<float>(settings.font_size),
        .base_scale = base_scale,
        .ascender = face->ascender,
        .descender = face->descender
    };
}

sge::font_loader::FontDataSDF sge::font_loader::LoadSdfFontDataFromFile(
    const std::string& path,
    const SdfSettings& settings
) {
    std::vector<uint8_t> buffer;
    common::ReadEntireFile(path, buffer);
    return sge::font_loader::LoadSdfFontDataFromBytes(buffer, settings);
}