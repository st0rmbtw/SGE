#include <SGE/assert.hpp>
#include <SGE/math/math.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/types/font.hpp>

#include <glm/common.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

using Point = glm::i16vec2;

namespace {

constexpr int32_t PARTITION_HEIGHT = 120;

struct BezierCurve {
    Point p0;
    Point p1;
    Point p2;
};

struct BezierCurvePartition {
    uint32_t curve_offset = 0;
    uint32_t curve_count = 0;
};

struct UserData {
    std::vector<BezierCurve>& curves;
    FT_GlyphSlot glyph;
    Point prevPoint;
};

int SolveQuadratic(float A, float B, float C, float roots[2]) {
    float discriminant = B * B - 4.f * A * C;
    if (discriminant < 0) return 0;
    
    if (discriminant == 0) {
        roots[0] = -B / (2.f * A);
        return 1;
    }
    
    float sqrtD = std::sqrt(discriminant);
    roots[0] = (-B - sqrtD) / (2.f * A);
    roots[1] = (-B + sqrtD) / (2.f * A);
    return 2;
}

bool IntersectsHorizontalLine(const BezierCurve& curve, float c) {
    float p0 = curve.p0.y;
    float p1 = curve.p1.y;
    float p2 = curve.p2.y; 

    float A = p0 - 2.f * p1 + p2;
    float B = 2.f * (p1 - p0);
    float C = p0 - c;

    float roots[2];
    int numRoots = SolveQuadratic(A, B, C, roots);
    
    for (int i = 0; i < numRoots; ++i) {
        float t = roots[i];
        if (t >= 0.f && t <= 1.f) {
            return true;
        }
    }
    return false;
}

// Helper function for detecting if a bezier curve intersects with a horizontal band
bool IsCurveIntersectingBand(const BezierCurve& curve, float bottom, float top) {
    if ((top >= curve.p0.y && bottom <= curve.p0.y) || (top >= curve.p2.y && bottom <= curve.p2.y)) {
        return true;
    }

    if (IntersectsHorizontalLine(curve, top))    return true;
    if (IntersectsHorizontalLine(curve, bottom)) return true;

    return false;
}

} // namespace

sge::FontVector sge::LoadFontVector(const std::string& path, sge::RenderContext& context) {
    FT_Outline_Funcs callbacks;
    callbacks.shift = 0;
    callbacks.delta = 0;
    callbacks.move_to = [](const FT_Vector* to, void* user) -> int {
        UserData& data = *static_cast<UserData*>(user);
        data.prevPoint = Point(to->x,to->y);
        return 0;
    };
    callbacks.line_to = [](const FT_Vector* to, void* user) -> int {
        UserData& data = *static_cast<UserData*>(user);
        Point toPoint = Point(to->x, to->y);

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
        Point p1 = Point(control->x, control->y);
        Point p2 = Point(to->x, to->y);

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

    std::vector<BezierCurve> curves;
    std::vector<BezierCurve> partition_curves;
    std::vector<BezierCurvePartition> partitions;
    std::unordered_map<uint32_t, Glyph> glyphs;

    while (true) {
        FT_Load_Glyph(face, index, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE);
        if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) continue;

        auto data = UserData {
            .curves = curves,
            .glyph = face->glyph
        };

        const uint32_t curve_offset = curves.size();
        FT_Outline_Decompose(&face->glyph->outline, &callbacks, &data);
        const uint32_t curve_count = curves.size() - curve_offset;

        uint32_t partition_offset = 0;
        uint32_t partition_count = 0;

        if (curve_count > 0) {
            FT_BBox bbox;
            FT_Outline_Get_CBox(&face->glyph->outline, &bbox);

            const auto width = (bbox.xMax - bbox.xMin);
            
            partition_offset = partitions.size();
            
            int32_t y = bbox.yMax;
            while (y > bbox.yMin) {
                BezierCurvePartition partition;
                partition.curve_offset = partition_curves.size();

                for (uint32_t i = 0; i < curve_count; ++i) {
                    BezierCurve curve = curves[curve_offset + i];

                    const float bottom = y - PARTITION_HEIGHT * 1.5f;
                    const float top = y;
                
                    if (IsCurveIntersectingBand(curve, bottom, top)) {
                        curve.p0 = Point(curve.p0.x - data.glyph->metrics.horiBearingX, data.glyph->metrics.horiBearingY - curve.p0.y);
                        curve.p1 = Point(curve.p1.x - data.glyph->metrics.horiBearingX, data.glyph->metrics.horiBearingY - curve.p1.y);
                        curve.p2 = Point(curve.p2.x - data.glyph->metrics.horiBearingX, data.glyph->metrics.horiBearingY - curve.p2.y);
                        partition_curves.push_back(curve);
                    }
                }

                partition.curve_count = (partition_curves.size() - partition.curve_offset);
                partitions.push_back(partition);

                y -= PARTITION_HEIGHT;
            }
            partition_count = partitions.size() - partition_offset;

            curves.clear();
        }

        glyphs.try_emplace(character, Glyph {
            .data = {
                .vector = GlyphDataVector {
                    .partition_offset = partition_offset,
                    .partition_count = partition_count,
                },
            },
            .size = glm::ivec2(face->glyph->metrics.width, face->glyph->metrics.height),
            .bearing = glm::ivec2(face->glyph->metrics.horiBearingX, face->glyph->metrics.horiBearingY),
            .advance = face->glyph->advance.x,
        });

        if (!index) break;

        character = FT_Get_Next_Char(face, character, &index);
    };

    return sge::FontVector {
        .glyphs = glyphs,
        .curve_buffer = context.CreateStructuredBuffer<BezierCurve>(partition_curves.size(), partition_curves.data()),
        .partition_buffer = context.CreateStructuredBuffer<BezierCurvePartition>(partitions.size(), partitions.data()),
        .units_per_em = face->units_per_EM,
        .ascender = static_cast<int16_t>(face->ascender),
        .descender = static_cast<int16_t>(face->descender),
    };
}

