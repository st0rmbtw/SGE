#include <fstream>

#include <SGE/assert.hpp>
#include <SGE/math/math.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/types/font.hpp>

#include <glm/common.hpp>

#include <ft2build.h>
#include <limits>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

using Point = glm::i16vec2;

namespace {

constexpr int32_t PARTITION_HEIGHT = 75;

struct BezierCurve {
    Point p0;
    Point p1;
    Point p2;
};

struct BezierGlyphPartition {
    uint32_t curve_offset = 0;
    uint32_t curve_count = 0;
};

struct UserData {
    std::vector<BezierCurve>& curves;
    Point prevPoint = Point(0, 0);
};

float SolveMonotonicBezierY(const BezierCurve& c, float target) {
    float p0 = c.p0.y, p1 = c.p1.y, p2 = c.p2.y;
    float qa = p0 - 2.f * p1 + p2;
    if (std::abs(qa) < 1e-3f) return (target - p0) / (p2 - p0);

    float qb = 2.f * (p1 - p0);
    float qc = p0 - target;
    float d = std::max(0.f, qb * qb - 4.f * qa * qc);
    float inv_2a = 0.5f / qa;
    float s = (p2 - p0) >= 0.f ? 1.f : -1.f;
    return -qb * inv_2a + s * std::sqrt(d) * inv_2a;
}

std::pair<BezierCurve, BezierCurve> SplitCurve(const BezierCurve& c, float t) {
    glm::vec2 q0    = glm::mix(glm::vec2(c.p0), glm::vec2(c.p1), t);
    glm::vec2 q1    = glm::mix(glm::vec2(c.p1), glm::vec2(c.p2), t);
    glm::vec2 split = glm::mix(q0, q1, t);

    auto toPoint = [](glm::vec2 v) -> Point {
        return Point(static_cast<Point::value_type>(std::lround(v.x)),
                    static_cast<Point::value_type>(std::lround(v.y)));
    };

    return {
        BezierCurve{ .p0=toPoint(c.p0),  .p1=toPoint(q0), .p2=toPoint(split) },
        BezierCurve{ .p0=toPoint(split), .p1=toPoint(q1), .p2=toPoint(c.p2) }
    };
}

void SplitAtPartitionBoundaries(BezierCurve curve, float yMax, int totalBands, std::vector<std::pair<BezierCurve, int>>& out) {
    Point::value_type top = std::max(curve.p0.y, curve.p2.y);
    Point::value_type bot = std::min(curve.p0.y, curve.p2.y);

    int bandTop = std::clamp((int)std::floor((yMax - top) / PARTITION_HEIGHT), 0, totalBands - 1);
    int bandBot = std::clamp((int)std::floor((yMax - bot) / PARTITION_HEIGHT), 0, totalBands - 1);

    if (bandTop == bandBot) {
        out.emplace_back(curve, bandTop);
        return;
    }

    BezierCurve remaining = curve;

    if (curve.p2.y > curve.p0.y) {
        // p0 (t=0) sits near bandBot (low y); p2 (t=1) sits near bandTop (high y).
        for (int band = bandBot; band > bandTop; --band) {
            float boundaryY = yMax - band * PARTITION_HEIGHT; // upper edge of band
            float t = std::clamp(SolveMonotonicBezierY(remaining, boundaryY), 0.f, 1.f);
            auto [a, b] = SplitCurve(remaining, t); // a = [p0..split], b = [split..p2]
            out.emplace_back(a, band);
            remaining = b;
        }
        out.emplace_back(remaining, bandTop);
    } else {
        // p0 (t=0) sits near bandTop (high y); p2 (t=1) sits near bandBot (low y).
        for (int band = bandTop; band < bandBot; ++band) {
            float boundaryY = yMax - (band + 1) * PARTITION_HEIGHT; // lower edge of band
            float t = std::clamp(SolveMonotonicBezierY(remaining, boundaryY), 0.f, 1.f);
            auto [a, b] = SplitCurve(remaining, t);
            out.emplace_back(a, band);
            remaining = b;
        }
        out.emplace_back(remaining, bandBot);
    }
}

} // namespace

sge::FontVector sge::LoadFontVectorFromBytes(std::span<const uint8_t> buffer, sge::RenderContext& context) {
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

        glm::vec2 pa = glm::vec2(p0 + p2 - static_cast<Point::value_type>(2) * p1);

        bool curve_emitted = false;

        // Checking if the x inflection point is defined
        if (pa.x > std::numeric_limits<glm::vec2::value_type>::epsilon()) {
            // Finding the x inflection point on the curve
            float inflection_x = (p0.x - p1.x) / pa.x;

            // Splitting the curve at the x inflection point
            if (inflection_x > 0.0f && inflection_x < 1.0f) {
                Point ctrl1 = glm::mix(glm::vec2(p0), glm::vec2(p1), inflection_x);
                Point ctrl2 = glm::mix(glm::vec2(p1), glm::vec2(p2), inflection_x);

                data.curves.emplace_back(p0, ctrl1, p1);
                data.curves.emplace_back(p1, ctrl2, p2);
                data.prevPoint = p2;

                curve_emitted = true;
            }
        }

        p0 = data.prevPoint;

        // Checking if the y inflection point is defined
        if (pa.y > std::numeric_limits<glm::vec2::value_type>::epsilon()) {
            // Finding the y inflection point on the curve
            float inflection_y = (p0.y - p1.y) / pa.y;

            // Splitting the curve at the y inflection point
            if (inflection_y > 0.0f && inflection_y < 1.0f) {
                Point ctrl1 = glm::mix(glm::vec2(p0), glm::vec2(p1), inflection_y);
                Point ctrl2 = glm::mix(glm::vec2(p1), glm::vec2(p2), inflection_y);

                data.curves.emplace_back(p0, ctrl1, p1);
                data.curves.emplace_back(p1, ctrl2, p2);
                data.prevPoint = p2;

                curve_emitted = true;
            }
        }

        if (!curve_emitted) {
            data.curves.emplace_back(p0, p1, p2);
            data.prevPoint = p2;
        }

        return 0;
    };
    callbacks.cubic_to = [](const FT_Vector*, const FT_Vector*, const FT_Vector*, void*) -> int {
        SGE_UNREACHABLE();
        return 0;
    };

    FT_Library library;
    FT_Init_FreeType(&library);

    FT_Face face;
    FT_New_Memory_Face(library, buffer.data(), buffer.size(), 0, &face);
    
    FT_UInt index;
    FT_ULong character = FT_Get_First_Char(face, &index);

    std::vector<BezierCurve> curves;
    std::vector<BezierCurve> partition_curves;
    std::vector<BezierGlyphPartition> partitions;
    std::unordered_map<uint32_t, Glyph> glyphs;

    std::vector<std::pair<BezierCurve, int>> pieces;
    std::vector<std::vector<BezierCurve>> bands;

    while (true) {
        FT_Load_Glyph(face, index, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE);
        if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) continue;

        auto data = UserData {
            .curves = curves
        };

        const uint32_t curve_offset = curves.size();
        FT_Outline_Decompose(&face->glyph->outline, &callbacks, &data);
        const uint32_t curve_count = curves.size() - curve_offset;

        uint32_t partition_offset = 0;

        int total_bands = 0;

        if (curve_count > 0) {
            FT_BBox bbox;
            FT_Outline_Get_CBox(&face->glyph->outline, &bbox);

            total_bands = std::max(1, (int)std::ceil((bbox.yMax - bbox.yMin) / (float)PARTITION_HEIGHT));

            for (uint32_t i = 0; i < curve_count; ++i) {
                SplitAtPartitionBoundaries(curves[curve_offset + i], (float)bbox.yMax, total_bands, pieces);
            }

            bands.resize(total_bands);
            for (auto& [piece, band] : pieces) {
                BezierCurve t = piece;
                t.p0 = Point(piece.p0.x - face->glyph->metrics.horiBearingX, face->glyph->metrics.horiBearingY - piece.p0.y);
                t.p1 = Point(piece.p1.x - face->glyph->metrics.horiBearingX, face->glyph->metrics.horiBearingY - piece.p1.y);
                t.p2 = Point(piece.p2.x - face->glyph->metrics.horiBearingX, face->glyph->metrics.horiBearingY - piece.p2.y);
                bands[band].push_back(t);
            }

            partition_offset = partitions.size();
            for (int band = 0; band < total_bands; ++band) {
                BezierGlyphPartition partition;
                partition.curve_offset = partition_curves.size();
                partition_curves.insert(partition_curves.end(), bands[band].begin(), bands[band].end());
                partition.curve_count = bands[band].size();
                partitions.push_back(partition);
            }

            curves.clear();
            pieces.clear();
            bands.clear();
        }

        glyphs.try_emplace(character, Glyph {
            .data = {
                .vector = GlyphDataVector {
                    .partition_offset = partition_offset,
                    .partition_count = static_cast<uint32_t>(total_bands),
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
        .partition_buffer = context.CreateStructuredBuffer<BezierGlyphPartition>(partitions.size(), partitions.data()),
        .units_per_em = face->units_per_EM,
        .ascender = static_cast<int16_t>(face->ascender),
        .descender = static_cast<int16_t>(face->descender),
    };
}

sge::FontVector sge::LoadFontVector(const std::string& path, sge::RenderContext& context) {
    std::ifstream file(path, std::ios::binary);
    assert(!file.fail());

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    bool failed = file.read(reinterpret_cast<char*>(buffer.data()), size).fail();
    assert(!failed);

    return LoadFontVectorFromBytes(buffer, context);
}

