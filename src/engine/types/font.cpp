#include <algorithm>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <limits>

#include <SGE/assert.hpp>
#include <SGE/math/math.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/types/font.hpp>
#include <SGE/utils/containers/heaparray.hpp>

#include "rectpack2D/empty_spaces.h"
#include "rectpack2D/finders_interface.h"

#include <glm/common.hpp>
#include <glm/gtx/norm.hpp>

#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_OUTLINE_H

using Point = glm::i16vec2;

namespace {

constexpr int32_t PARTITION_HEIGHT = 75;

struct CubicBezier {
    Point p0;
    Point p1;
    Point p2;
    Point p3;

    glm::vec2 sample(float t) const {
        float mt = 1.f - t;
        return glm::vec2(p0) * (mt * mt * mt) + 
               glm::vec2(p1) * (3.f * mt * mt * t) + 
               glm::vec2(p2) * (3.f * mt * t * t) + 
               glm::vec2(p3) * (t * t * t);
    }
};

struct QuadraticBezier {
    Point p0;
    Point p1;
    Point p2;
};

struct BezierGlyphPartition {
    uint32_t curve_offset = 0;
    uint32_t curve_count = 0;
};

class CurveList {
public:
    CurveList() = default;

    void push_curve(const QuadraticBezier& curve) {
        m_curves.push_back(curve);
        m_prev_point = curve.p2;
    }

    void set_prev_point(Point p) {
        m_prev_point = p;
    }

    void clear() {
        m_curves.clear();
    }

    void reverse() {
        for (auto& curve : m_curves) {
            std::swap(curve.p0, curve.p2);
        }
    }

    // Determines contour orientation: returns > 0 for CCW, < 0 for CW
    int64_t signed_area() {
        int64_t total_sum = 0;

        for (const auto& curve : m_curves) {
            // Shoelace contribution from endpoints + control point adjustment
            // Derived via Green's theorem for quadratic bezier parametrization
            total_sum += (curve.p0.x * curve.p2.y - curve.p2.x * curve.p0.y) + 
                (curve.p0.x * curve.p1.y - curve.p1.x * curve.p0.y) + 
                (curve.p1.x * curve.p2.y - curve.p2.x * curve.p1.y);
        }

        return total_sum;
    }

    [[nodiscard]]
    Point prev_point() const noexcept {
        return m_prev_point;
    }

    [[nodiscard]]
    size_t size() const noexcept {
        return m_curves.size();
    }

    const QuadraticBezier& operator[](size_t index) const {
        return m_curves[index];
    }

private:
    std::vector<QuadraticBezier> m_curves;
    Point m_prev_point = Point(0, 0);
};

float SolveMonotonicBezierY(const QuadraticBezier& c, float target) {
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

std::pair<CubicBezier, CubicBezier> SplitCubic(const CubicBezier& c, float t) {
    glm::vec2 p0_1 = glm::mix(glm::vec2(c.p0), glm::vec2(c.p1), t);
    glm::vec2 p1_1 = glm::mix(glm::vec2(c.p1), glm::vec2(c.p2), t);
    glm::vec2 p2_1 = glm::mix(glm::vec2(c.p2), glm::vec2(c.p3), t);
    glm::vec2 p0_2 = glm::mix(p0_1, p1_1, t);
    glm::vec2 p1_2 = glm::mix(p1_1, p2_1, t);
    glm::vec2 p0_3 = glm::mix(p0_2, p1_2, t);

    auto toPoint = [](glm::vec2 v) -> Point {
        return Point(static_cast<Point::value_type>(std::lround(v.x)),
                    static_cast<Point::value_type>(std::lround(v.y)));
    };

    return {
        CubicBezier { .p0=c.p0, .p1=toPoint(p0_1), .p2=toPoint(p0_2), .p3=toPoint(p0_3) },
        CubicBezier { .p0=toPoint(p0_3), .p1=toPoint(p1_2), .p2=toPoint(p2_1), .p3=c.p3 }
    };
}

std::pair<QuadraticBezier, QuadraticBezier> SplitQuadratic(const QuadraticBezier& c, float t) {
    glm::vec2 q0    = glm::mix(glm::vec2(c.p0), glm::vec2(c.p1), t);
    glm::vec2 q1    = glm::mix(glm::vec2(c.p1), glm::vec2(c.p2), t);
    glm::vec2 split = glm::mix(q0, q1, t);

    auto toPoint = [](glm::vec2 v) -> Point {
        return Point(static_cast<Point::value_type>(std::lround(v.x)),
                    static_cast<Point::value_type>(std::lround(v.y)));
    };

    return {
        QuadraticBezier{ .p0=toPoint(c.p0),  .p1=toPoint(q0), .p2=toPoint(split) },
        QuadraticBezier{ .p0=toPoint(split), .p1=toPoint(q1), .p2=toPoint(c.p2) }
    };
}

QuadraticBezier TryApproximate(const CubicBezier& c) {
    QuadraticBezier q;
    q.p0 = c.p0;
    q.p2 = c.p3;
    // Derive control point using intersecting tangent vectors
    q.p1 = glm::vec2(c.p0) + glm::vec2(c.p1 - c.p0) * 1.5f; 
    return q;
}

// Measures max deviation between the cubic curve and a quadratic segment at t = 0.5
float EvaluateError(const CubicBezier& c, const QuadraticBezier& q) {
    glm::vec2 cubicMid = c.sample(0.5f);
    glm::vec2 quadMid = glm::vec2(q.p0) * 0.25f + glm::vec2(q.p1) * 0.5f + glm::vec2(q.p2) * 0.25f; 
    return glm::distance2(cubicMid, quadMid);
}

// Recursive function mapping the cubic path to a series of quadratic curves
void CubicToQuadratic(const CubicBezier& c, float toleranceSq, CurveList& out) {
    QuadraticBezier q = TryApproximate(c);
    
    if (EvaluateError(c, q) <= toleranceSq) {
        out.push_curve(q);
    } else {
        auto [left, right] = SplitCubic(c, toleranceSq);
        CubicToQuadratic(left, toleranceSq, out);
        CubicToQuadratic(right, toleranceSq, out);
    }
}

void SplitAtPartitionBoundaries(QuadraticBezier curve, float yMax, int totalBands, std::vector<std::pair<QuadraticBezier, int>>& out) {
    Point::value_type top = std::max(curve.p0.y, curve.p2.y);
    Point::value_type bot = std::min(curve.p0.y, curve.p2.y);

    int bandTop = std::clamp((int)std::floor((yMax - top) / PARTITION_HEIGHT), 0, totalBands - 1);
    int bandBot = std::clamp((int)std::floor((yMax - bot) / PARTITION_HEIGHT), 0, totalBands - 1);

    if (bandTop == bandBot) {
        out.emplace_back(curve, bandTop);
        return;
    }

    QuadraticBezier remaining = curve;

    if (curve.p2.y > curve.p0.y) {
        // p0 (t=0) sits near bandBot (low y); p2 (t=1) sits near bandTop (high y).
        for (int band = bandBot; band > bandTop; --band) {
            float boundaryY = yMax - band * PARTITION_HEIGHT; // upper edge of band
            float t = std::clamp(SolveMonotonicBezierY(remaining, boundaryY), 0.f, 1.f);
            auto [a, b] = SplitQuadratic(remaining, t); // a = [p0..split], b = [split..p2]
            out.emplace_back(a, band);
            remaining = b;
        }
        out.emplace_back(remaining, bandTop);
    } else {
        // p0 (t=0) sits near bandTop (high y); p2 (t=1) sits near bandBot (low y).
        for (int band = bandTop; band < bandBot; ++band) {
            float boundaryY = yMax - (band + 1) * PARTITION_HEIGHT; // lower edge of band
            float t = std::clamp(SolveMonotonicBezierY(remaining, boundaryY), 0.f, 1.f);
            auto [a, b] = SplitQuadratic(remaining, t);
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
        CurveList& curves = *static_cast<CurveList*>(user);
        curves.set_prev_point(Point(to->x,to->y));
        return 0;
    };
    callbacks.line_to = [](const FT_Vector* to, void* user) -> int {
        CurveList& curves = *static_cast<CurveList*>(user);
        Point toPoint = Point(to->x, to->y);

        // Skip horizontal lines
        if (!sge::ApproxEquals(curves.prev_point().y, toPoint.y)) {
            curves.push_curve(QuadraticBezier{ curves.prev_point(), toPoint, toPoint });
        }
        curves.set_prev_point(toPoint);

        return 0;
    };
    callbacks.conic_to = [](const FT_Vector* control, const FT_Vector* to, void* user) -> int {
        CurveList& curves = *static_cast<CurveList*>(user);
        Point p0 = curves.prev_point();
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

                curves.push_curve(QuadraticBezier{ p0, ctrl1, p1 });
                curves.push_curve(QuadraticBezier{ p1, ctrl2, p2 });

                curve_emitted = true;
            }
        }

        p0 = curves.prev_point();

        // Checking if the y inflection point is defined
        if (pa.y > std::numeric_limits<glm::vec2::value_type>::epsilon()) {
            // Finding the y inflection point on the curve
            float inflection_y = (p0.y - p1.y) / pa.y;

            // Splitting the curve at the y inflection point
            if (inflection_y > 0.0f && inflection_y < 1.0f) {
                Point ctrl1 = glm::mix(glm::vec2(p0), glm::vec2(p1), inflection_y);
                Point ctrl2 = glm::mix(glm::vec2(p1), glm::vec2(p2), inflection_y);

                curves.push_curve(QuadraticBezier{ p0, ctrl1, p1 });
                curves.push_curve(QuadraticBezier{ p1, ctrl2, p2 });

                curve_emitted = true;
            }
        }

        if (!curve_emitted) {
            curves.push_curve(QuadraticBezier{ p0, p1, p2 });
        }

        return 0;
    };
    callbacks.cubic_to = [](const FT_Vector* ctrl1, const FT_Vector* ctrl2, const FT_Vector* to, void* user) -> int {
        CurveList& curves = *static_cast<CurveList*>(user);

        const auto curve = CubicBezier {
            .p0=curves.prev_point(),
            .p1=Point(ctrl1->x, ctrl1->y),
            .p2=Point(ctrl2->x, ctrl2->y),
            .p3=Point(to->x, to->y)
        };

        glm::vec2 a = -1.f * glm::vec2(curve.p0) + 3.f * glm::vec2(curve.p1) - 3.f * glm::vec2(curve.p2) + glm::vec2(curve.p3);
        glm::vec2 b =  3.f * glm::vec2(curve.p0) - 6.f * glm::vec2(curve.p1) + 3.f * glm::vec2(curve.p2);
        glm::vec2 c = -3.f * glm::vec2(curve.p0) + 3.f * glm::vec2(curve.p1);

        glm::vec2 d = b*b - 3.f * a * c;

        float valid_ts[4]{ 0.f, 0.f, 0.f, 0.f };
        uint8_t n = 0;

        if (d.x >= 0.f) {
            float sqrt_d = std::sqrtf(d.x);
            float t1 = (-b.x - sqrt_d) / (3.f * a.x);
            float t2 = (-b.x + sqrt_d) / (3.f * a.x);

            // Filter roots that lie strictly inside the curve boundary
            if (t1 > 1e-6f && t1 < (1.f - 1e-6f))
                valid_ts[n++] = t1;
            if (t2 > 1e-6f && t2 < (1.f - 1e-6f))
                valid_ts[n++] = t2;
        }

        if (d.y >= 0.f) {
            float sqrt_d = std::sqrtf(d.y);
            float t1 = (-b.y - sqrt_d) / (3.f * a.y);
            float t2 = (-b.y + sqrt_d) / (3.f * a.y);

            // Filter roots that lie strictly inside the curve boundary
            if (t1 > 1e-6f && t1 < (1.f - 1e-6f))
                valid_ts[n++] = t1;
            if (t2 > 1e-6f && t2 < (1.f - 1e-6f))
                valid_ts[n++] = t2;
        }

        std::sort(valid_ts, valid_ts + n);

        auto current_remaining = curve;
        float last_t = 0.f;

        for (auto i = 0; i < n; ++i) {
            float t = valid_ts[i];
            // Map global t to the localized scale of the remaining sub-curve
            float local_t = (t - last_t) / (1.0 - last_t);
            
            auto [left, right] = SplitCubic(current_remaining, local_t);
            CubicToQuadratic(left, 0.5f, curves);
            current_remaining = right;
            
            last_t = t;
        }
        
        CubicToQuadratic(current_remaining, 0.5f, curves);

        return 0;
    };

    FT_Library library;
    FT_Init_FreeType(&library);

    FT_Face face;
    FT_New_Memory_Face(library, buffer.data(), buffer.size(), 0, &face);
    
    FT_UInt index;
    FT_ULong character = FT_Get_First_Char(face, &index);

    CurveList curves;
    std::vector<QuadraticBezier> partition_curves;
    std::vector<BezierGlyphPartition> partitions;
    std::unordered_map<uint32_t, Glyph> glyphs;

    std::vector<std::pair<QuadraticBezier, int>> pieces;
    std::vector<std::vector<QuadraticBezier>> bands;

    while (true) {
        FT_Load_Glyph(face, index, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE);
        if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) continue;

        FT_Outline_Decompose(&face->glyph->outline, &callbacks, &curves);
        const uint32_t curve_count = curves.size();

        uint32_t partition_offset = 0;

        int total_bands = 0;

        if (curve_count > 0) {
            auto sum = curves.signed_area();
            // The winding order is clockwise, converting it to counter-clockwise
            if (sum < 0)
                curves.reverse();
            else {
                (void)0;
            };

            FT_BBox bbox;
            FT_Outline_Get_CBox(&face->glyph->outline, &bbox);

            total_bands = std::max(1, (int)std::ceil((bbox.yMax - bbox.yMin) / (float)PARTITION_HEIGHT));

            for (uint32_t i = 0; i < curve_count; ++i) {
                SplitAtPartitionBoundaries(curves[i], (float)bbox.yMax, total_bands, pieces);
            }

            bands.resize(total_bands);
            for (auto& [piece, band] : pieces) {
                QuadraticBezier t = piece;
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
        .curve_buffer = context.CreateStructuredBuffer<QuadraticBezier>(partition_curves.size(), partition_curves.data()),
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

namespace {
    constexpr uint32_t SDF_FONT_SIZE = 96;
    constexpr uint32_t SDF_PADDING = 2;

    struct GlyphInfo {
        sge::HeapArray<uint8_t> buffer;
        FT_ULong character;
        uint32_t width;
        uint32_t height;
        FT_Int bitmap_left;
        FT_Int bitmap_top;
        FT_Pos advance_x;
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

sge::Font sge::LoadFontFromBytes(std::span<const uint8_t> buffer, class sge::RenderContext& context) {
    FT_Library library;
    FT_Init_FreeType(&library);

    FT_Face face;
    FT_New_Memory_Face(library, buffer.data(), buffer.size(), 0, &face);
    FT_Set_Pixel_Sizes(face, 0, SDF_FONT_SIZE);
    
    FT_UInt index;
    FT_ULong character = FT_Get_First_Char(face, &index);

    FT_Int max_ascent = std::numeric_limits<FT_Int>::min();
    FT_Int max_descent = std::numeric_limits<FT_Int>::min();

    std::vector<GlyphInfo> glyphs;
    
    while (true) {
        FT_Load_Char(face, character, FT_LOAD_DEFAULT);
        FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF);

        FT_Bitmap* bitmap = &face->glyph->bitmap;
        
        sge::HeapArray<uint8_t> buffer(static_cast<size_t>(bitmap->rows) * bitmap->width);
        for (uint32_t y = 0; y < bitmap->rows; ++y) {
            uint8_t* row_data = bitmap->buffer + y * bitmap->pitch;
            for (uint32_t x = 0; x < bitmap->width; ++x) {
                const size_t idx = y * bitmap->width + x;
                buffer[idx] = row_data[x];
            }
        }

        max_ascent = std::max(max_ascent, face->glyph->bitmap_top);
        max_descent = std::max(max_descent, FT_Int(bitmap->rows) - face->glyph->bitmap_top);

        glyphs.push_back(GlyphInfo{
            .buffer = std::move(buffer),
            .character = character,
            .width = bitmap->width,
            .height = bitmap->rows,
            .bitmap_left = face->glyph->bitmap_left,
            .bitmap_top = face->glyph->bitmap_top,
            .advance_x = face->glyph->advance.x,
            .col = 0,
            .row = 0
        });
        
        if (!index) break;

        character = FT_Get_Next_Char(face, character, &index);
    }

    std::vector<rect> rectangles;
    for (size_t i = 0; i < glyphs.size(); ++i) {
        const GlyphInfo& g = glyphs[i];
        rectangles.emplace_back(rectpack2D::rect_xywh(0, 0, g.width + SDF_PADDING*2, g.height + SDF_PADDING*2), i);
    }

    const LLGL::RenderingLimits& limits = context.GetRenderingCaps().limits;

    const auto result_size = find_best_packing<spaces_type>(
        rectangles,
        make_finder_input(
            rectpack2D::rect_wh(limits.max2DTextureSize, limits.max2DTextureSize),
            -4,
            [](rect_type&) { return rectpack2D::callback_result::CONTINUE_PACKING; },
            [](rect_type&) {
                SGE_UNREACHABLE();
                return rectpack2D::callback_result::ABORT_PACKING;
            },
            rectpack2D::flipping_option::DISABLED
        )
    );

    for (auto& rect : rectangles) {
        glyphs[rect.index].col = rect.xywh.x + SDF_PADDING;
        glyphs[rect.index].row = rect.xywh.y + SDF_PADDING;
    }

    const uint32_t texture_width = result_size.w;
    const uint32_t texture_height = result_size.h;

    sge::HeapArray<uint8_t> texture_data(texture_width * texture_height, 0x0);
    for (const auto& g : glyphs) {
        for (uint32_t y = 0; y < g.height; ++y) {
            size_t dst_idx = (g.row + y) * texture_width + g.col;
            size_t src_idx = y * g.width;
            memcpy(&texture_data[dst_idx], &g.buffer[src_idx], g.width);
        }
    }

    std::unordered_map<uint32_t, sge::Glyph> glyph_map;
    for (const auto& g : glyphs) {
        float tex_width = float(g.width) / texture_width;
        float tex_height = float(g.height) / texture_height;
        float texture_coord_x = float(g.col) / texture_width;
        float texture_coord_y = float(g.row) / texture_height;
        glyph_map[g.character] = sge::Glyph {
            .data = {
                .sdf = {
                    .tex_size = glm::vec2(tex_width, tex_height),
                    .texture_coords = glm::vec2(texture_coord_x, texture_coord_y),
                }
            },
            .size = glm::ivec2(g.width, g.height),
            .bearing = glm::ivec2(g.bitmap_left, g.bitmap_top),
            .advance = g.advance_x
        };
    }

    sge::TextureConfig textureConfig;
    textureConfig.debugName = "Default Font Texture";
    textureConfig.textureType = LLGL::TextureType::Texture2D;
    textureConfig.extent.width = texture_width;
    textureConfig.extent.height = texture_height;
    textureConfig.extent.depth = 1;
    textureConfig.sampler = context.GetLinearSampler();

    LLGL::ImageView imageView;
    imageView.dataType = LLGL::DataType::UInt8;
    imageView.format = LLGL::ImageFormat::R;
    imageView.dataSize = textureConfig.extent.width * textureConfig.extent.height;
    imageView.data = texture_data.data();

    return sge::Font {
        .glyphs = std::move(glyph_map),
        .texture = context.CreateTexture(textureConfig, &imageView),
        .font_size = SDF_FONT_SIZE,
        .max_ascent = static_cast<float>(max_ascent),
        .max_descent = static_cast<float>(max_descent),
        .ascender = static_cast<int16_t>(face->ascender >> 6),
    };
}

sge::Font sge::LoadFont(const std::string& path, sge::RenderContext& context) {
    std::ifstream file(path, std::ios::binary);
    assert(!file.fail());

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    bool failed = file.read(reinterpret_cast<char*>(buffer.data()), size).fail();
    assert(!failed);

    return LoadFontFromBytes(buffer, context);
}