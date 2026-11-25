#include <SGE/font_loader.hpp>
#include <SGE/types/color.hpp>
#include <SGE/types/path_builder.hpp>

#include <SGE/types/vector_shape.hpp>
#include <glm/common.hpp>
#include <glm/gtx/norm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
#include FT_OUTLINE_H
#include FT_ADVANCES_H
#include FT_TRUETYPE_TABLES_H

#include "common.hpp"

sge::font_loader::FontDataVector sge::font_loader::LoadVectorFontDataFromBytes(std::span<const uint8_t> buffer) {
    FT_Outline_Funcs callbacks;
    callbacks.shift = 0;
    callbacks.delta = 0;
    callbacks.move_to = [](const FT_Vector* to, void* user) -> int {
        PathBuilder& builder = *static_cast<PathBuilder*>(user);
        builder.MoveTo({ to->x, to->y });
        return 0;
    };
    callbacks.line_to = [](const FT_Vector* to, void* user) -> int {
        PathBuilder& builder = *static_cast<PathBuilder*>(user);
        builder.LineTo({ to->x, to->y });
        return 0;
    };
    callbacks.conic_to = [](const FT_Vector* ctrl, const FT_Vector* to, void* user) -> int {
        PathBuilder& builder = *static_cast<PathBuilder*>(user);
        builder.QuadraticBezierTo({ ctrl->x, ctrl->y }, { to->x, to->y });
        return 0;
    };
    callbacks.cubic_to = [](const FT_Vector* ctrl1, const FT_Vector* ctrl2, const FT_Vector* to, void* user) -> int {
        PathBuilder& builder = *static_cast<PathBuilder*>(user);
        builder.CubicBezierTo({ ctrl1->x, ctrl1->y }, { ctrl2->x, ctrl2->y }, { to->x, to->y });
        return 0;
    };

    FT_Library library;
    FT_Init_FreeType(&library);

    FT_Face face;
    FT_New_Memory_Face(library, buffer.data(), buffer.size(), 0, &face);

    FT_UInt index;
    FT_ULong character = FT_Get_First_Char(face, &index);

    std::vector<GlyphInfoVector> glyphs;
    std::vector<VectorShapeLayer> layers;
    PathBuilder pathBuilder;

    FT_Short xHeight = 0;

    while (true) {
        FT_Load_Glyph(face, index, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE);
        if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) continue;

        if (character == 'x') {
            xHeight = face->glyph->metrics.height;
        }

        pathBuilder.Reset();
        FT_Outline_Decompose(&face->glyph->outline, &callbacks, &pathBuilder);

        VectorShapeLayerId layerId = VectorShapeLayerId::INVALID;
        if (!pathBuilder.IsEmpty()) {
            layerId = VectorShapeLayerId{ static_cast<uint32_t>(layers.size()) };
            layers.push_back(VectorShapeLayer{
                .path = pathBuilder.Build(),
                .color = sge::color::WHITE,
                .customSize = glm::vec2(face->glyph->metrics.width, face->glyph->metrics.height)
            });
        }

        glyphs.push_back(GlyphInfoVector {
            .size = glm::ivec2(face->glyph->metrics.width, face->glyph->metrics.height),
            .bearing = glm::ivec2(face->glyph->metrics.horiBearingX, face->glyph->metrics.horiBearingY),
            .character = character,
            .layerId = layerId,
            .advance = float(face->glyph->advance.x),
        });

        if (!index) break;

        character = FT_Get_Next_Char(face, character, &index);
    };

    // Try to get x-height from OS/2
    TT_OS2* os2 = (TT_OS2*)FT_Get_Sfnt_Table(face, FT_SFNT_OS2);
    if (os2 && os2->sxHeight != 0) {
        xHeight = os2->sxHeight;
    }

    return FontDataVector {
        .glyphs = std::move(glyphs),
        .layers = std::move(layers),
        .units_per_em = face->units_per_EM,
        .ascender = static_cast<int16_t>(face->ascender),
        .descender = static_cast<int16_t>(face->descender),
        .xHeight = xHeight
    };
}

sge::font_loader::FontDataVector sge::font_loader::LoadVectorFontDataFromFile(const std::string& path) {
    std::vector<uint8_t> buffer;
    common::ReadEntireFile(path, buffer);
    return sge::font_loader::LoadVectorFontDataFromBytes(buffer);
}
