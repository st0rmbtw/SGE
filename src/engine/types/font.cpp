#include <SGE/assert.hpp>
#include <SGE/math/math.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/types/font.hpp>

#include <algorithm>
#include <algorithm>
#include <glm/common.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

using Point = glm::i16vec2;

namespace {

struct BezierCurve {
    Point p0;
    Point p1;
    Point p2;
};

struct UserData {
    std::vector<BezierCurve>& curves;
    FT_GlyphSlot glyph;
    Point prevPoint;
};

} // namespace

sge::FontVector sge::LoadFontVector(const std::string& path, sge::RenderContext& context) {
    std::vector<BezierCurve> curves;
    std::unordered_map<uint32_t, Glyph> glyphs;

    FT_Outline_Funcs callbacks;
    callbacks.shift = 0;
    callbacks.delta = 0;
    callbacks.move_to = [](const FT_Vector* to, void* user) -> int {
        UserData& data = *static_cast<UserData*>(user);
        data.prevPoint = Point(to->x - data.glyph->metrics.horiBearingX, data.glyph->metrics.horiBearingY - to->y);
        return 0;
    };
    callbacks.line_to = [](const FT_Vector* to, void* user) -> int {
        UserData& data = *static_cast<UserData*>(user);
        Point toPoint = Point(to->x - data.glyph->metrics.horiBearingX, data.glyph->metrics.horiBearingY - to->y);

        // Skip horizontal lines
        if (!sge::ApproxEquals(data.prevPoint.y, toPoint.y)) {
            data.curves.emplace_back(data.prevPoint, toPoint, toPoint);
        }
        data.prevPoint = toPoint;

        return 0;
    };
    callbacks.conic_to = [](const FT_Vector* control, const FT_Vector* to, void* user) -> int {
        UserData& data = *static_cast<UserData*>(user);
        Point p0 = data.prevPoint;
        Point p1 = Point(control->x - data.glyph->metrics.horiBearingX, data.glyph->metrics.horiBearingY - control->y);
        Point p2 = Point(to->x - data.glyph->metrics.horiBearingX, data.glyph->metrics.horiBearingY - to->y);

        // Finding the x and y inflection points on the curve
        glm::vec2 inflection = glm::vec2(p0 - p1) / glm::vec2(p0 + p2 - static_cast<Point::value_type>(2) * p1);

        bool curve_emitted = false;

        // Splitting the curve at the x inflection point
        if (inflection.x > 0.0f && inflection.x < 1.0f) {
            Point ctrl1 = glm::mix(glm::vec2(p0), glm::vec2(p1), inflection.x);
            Point ctrl2 = glm::mix(glm::vec2(p1), glm::vec2(p2), inflection.x);

            data.curves.emplace_back(p0, ctrl1, p1);
            data.curves.emplace_back(p1, ctrl2, p2);
            data.prevPoint = p2;

            curve_emitted = true;
        }

        p0 = data.prevPoint;

        // Splitting the curve at the y inflection point
        if (inflection.y > 0.0f && inflection.y < 1.0f) {
            Point ctrl1 = glm::mix(glm::vec2(p0), glm::vec2(p1), inflection.y);
            Point ctrl2 = glm::mix(glm::vec2(p1), glm::vec2(p2), inflection.y);

            data.curves.emplace_back(p0, ctrl1, p1);
            data.curves.emplace_back(p1, ctrl2, p2);
            data.prevPoint = p2;

            curve_emitted = true;
        }

        if (!curve_emitted) {
            data.curves.emplace_back(p0, p1, p2);
            data.prevPoint = p2;
        }

        return 0;
    };
    callbacks.cubic_to = [](const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user) -> int {
        SGE_UNREACHABLE();
        return 0;
    };

    FT_Library library;
    FT_Init_FreeType(&library);

    FT_Face face;
    FT_New_Face(library, path.c_str(), 0, &face);
    
    FT_UInt index;
    FT_ULong character = FT_Get_First_Char(face, &index);

    while (true) {
        FT_Load_Glyph(face, index, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE);
        if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) continue;

        auto data = UserData {
            .curves = curves,
            .glyph = face->glyph
        };

        const size_t offset = curves.size();
        FT_Outline_Decompose(&face->glyph->outline, &callbacks, &data);
        const size_t count = curves.size() - offset;
        
        std::sort(
            std::next(curves.begin(), offset),
            std::next(curves.begin(), offset + count),
            [](const BezierCurve& a, const BezierCurve& b) -> bool {
                const float a_min_y = std::min({a.p0.y, a.p1.y, a.p2.y});
                const float b_min_y = std::min({b.p0.y, b.p1.y, b.p2.y});
                return a_min_y < b_min_y;
            }
        );

        glyphs.try_emplace(character, Glyph {
            .data = {
                .vector = GlyphDataVector {
                    .offset = offset,
                    .count = count,
                },
            },
            .size = glm::ivec2(face->glyph->metrics.width, face->glyph->metrics.height),
            .bearing = glm::ivec2(face->glyph->metrics.horiBearingX, face->glyph->metrics.horiBearingY),
            .advance = face->glyph->advance.x,
        });

        character = FT_Get_Next_Char(face, character, &index);
        if (!index) break;
    }

    return sge::FontVector {
        .glyphs = glyphs,
        .buffer = context.CreateStructuredBuffer<BezierCurve>(curves.size(), curves.data()),
        .units_per_em = face->units_per_EM,
        .ascender = static_cast<int16_t>(face->ascender),
        .descender = static_cast<int16_t>(face->descender),
    };
}

