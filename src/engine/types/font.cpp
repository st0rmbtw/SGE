#include <SGE/assert.hpp>
#include <SGE/math/math.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/types/font.hpp>

#include <glm/common.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

using Point = glm::vec2;

namespace {

struct UserData {
    std::vector<Point>& points;
    FT_GlyphSlot glyph;
};

} // namespace

sge::FontVector sge::LoadFontVector(const std::string& path, sge::RenderContext& context) {
    std::vector<Point> points;
    std::unordered_map<uint32_t, Glyph> glyphs;

    FT_Outline_Funcs callbacks;
    callbacks.shift = 0;
    callbacks.delta = 0;
    callbacks.move_to = [](const FT_Vector* to, void* user) -> int {
        UserData& data = *static_cast<UserData*>(user);
        data.points.emplace_back(to->x - data.glyph->metrics.horiBearingX, data.glyph->metrics.horiBearingY - to->y);
        return 0;
    };
    callbacks.line_to = [](const FT_Vector* to, void* user) -> int {
        UserData& data = *static_cast<UserData*>(user);
        Point prevPoint = data.points.back();
        Point toPoint = glm::vec2(to->x - data.glyph->metrics.horiBearingX, data.glyph->metrics.horiBearingY - to->y);

        if (data.points.size() % 3 == 0) {
            data.points.push_back(prevPoint);
        }

        data.points.push_back(toPoint);
        data.points.push_back(toPoint);
        return 0;
    };
    callbacks.conic_to = [](const FT_Vector* control, const FT_Vector* to, void* user) -> int {
        UserData& data = *static_cast<UserData*>(user);
        Point p0 = data.points.back();
        Point p1 = glm::vec2(control->x - data.glyph->metrics.horiBearingX, data.glyph->metrics.horiBearingY - control->y);
        Point p2 = glm::vec2(to->x - data.glyph->metrics.horiBearingX, data.glyph->metrics.horiBearingY - to->y);

        // Finding the inflection points on the x and y axes
        float yt_inflection = (p0.y - p1.y) / (p0.y + p2.y - 2.0f * p1.y);
        float xt_inflection = (p0.x - p1.x) / (p0.x + p2.x - 2.0f * p1.x);

        bool point_emitted = false;

        // Splitting the curve at the y inflection point
        if (yt_inflection > 0.0f && yt_inflection < 1.0f) {
            Point ctrl1 = glm::mix(p0, p1, yt_inflection);
            Point ctrl2 = glm::mix(p1, p2, yt_inflection);

            if (data.points.size() % 3 == 0) {
                data.points.push_back(p0);
            }

            data.points.push_back(ctrl1);
            data.points.push_back(p1);
            data.points.push_back(p1);
            data.points.push_back(ctrl2);
            data.points.push_back(p2);
            point_emitted = true;
        }

        // Splitting the curve at the x inflection point
        if (xt_inflection > 0.0f && xt_inflection < 1.0f) {
            Point ctrl1 = glm::mix(p0, p1, xt_inflection);
            Point ctrl2 = glm::mix(p1, p2, xt_inflection);

            if (data.points.size() % 3 == 0) {
                data.points.push_back(p0);
            }

            data.points.push_back(ctrl1);
            data.points.push_back(p1);
            data.points.push_back(p1);
            data.points.push_back(ctrl2);
            data.points.push_back(p2);
            point_emitted = true;
        }

        if (!point_emitted) {
            if (data.points.size() % 3 == 0) {
                data.points.push_back(p0);
            }

            data.points.push_back(p1);
            data.points.push_back(p2);
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
            .points = points,
            .glyph = face->glyph
        };

        const size_t offset = points.size();
        FT_Outline_Decompose(&face->glyph->outline, &callbacks, &data);
        const size_t count = points.size() - offset;

        glyphs.try_emplace(character, Glyph {
            .data.vector = GlyphDataVector {
                .offset = offset,
                .count = count,
            },
            .size = glm::ivec2(face->glyph->metrics.width, face->glyph->metrics.height),
            .bearing = glm::ivec2(face->glyph->metrics.horiBearingX, face->glyph->metrics.horiBearingY),
            .advance = face->glyph->advance.x,
        });

        character = FT_Get_Next_Char(face, character, &index);
        if (!index) break;
    }

    SGE_ASSERT(points.size() % 3 == 0);

    for (auto& point : points) {
        point /= face->units_per_EM;
    }

    sge::Ref<LLGL::Buffer> buffer = context.CreateStructuredBuffer<Point>(points.size(), LLGL::Format::RG32Float, points.data());

    return sge::FontVector {
        .glyphs = glyphs,
        .buffer = std::move(buffer),
        .units_per_em = face->units_per_EM,
        .ascender = static_cast<int16_t>(face->ascender),
        .descender = static_cast<int16_t>(face->descender),
    };
}

