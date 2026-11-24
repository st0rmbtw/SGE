#include <algorithm>
#include <glm/gtc/quaternion.hpp>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/gtx/norm.hpp>

#include <SGE/types/path.hpp>

#include "convert.hpp"

namespace {

using point_t = glm::vec2;

struct CubicBezier {
    point_t p0, p1, p2, p3;

    glm::vec2 Sample(float t) const {
        float mt = 1.f - t;
        return glm::vec2(p0) * (mt * mt * mt) +
                glm::vec2(p1) * (3.f * mt * mt * t) +
                glm::vec2(p2) * (3.f * mt * t * t) +
                glm::vec2(p3) * (t * t * t);
    }
};

struct QuadraticBezier {
    point_t p0, p1, p2;
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
    const glm::vec2 a = glm::mix(c.p0, c.p1, t);
    const glm::vec2 b = glm::mix(c.p1, c.p2, t);
    const glm::vec2 d = glm::mix(c.p2, c.p3, t);
    const glm::vec2 e = glm::mix(a, b, t);
    const glm::vec2 f = glm::mix(b, d, t);
    const glm::vec2 m = glm::mix(e, f, t);
    return {{c.p0, a, e, m}, {m, f, d, c.p3}};
}

std::pair<QuadraticBezier, QuadraticBezier> SplitQuadratic(const QuadraticBezier& q, float t) {
    const glm::vec2 a = glm::mix(q.p0, q.p1, t);
    const glm::vec2 b = glm::mix(q.p1, q.p2, t);
    const glm::vec2 m = glm::mix(a, b, t);
    return {{q.p0, a, m}, {m, b, q.p2}};
}

inline void ClampMonotone(QuadraticBezier& q) {
    q.p1.x = std::clamp(q.p1.x, std::min(q.p0.x, q.p2.x), std::max(q.p0.x, q.p2.x));
    q.p1.y = std::clamp(q.p1.y, std::min(q.p0.y, q.p2.y), std::max(q.p0.y, q.p2.y));
}

void SplitAtBandBoundaries(QuadraticBezier q, float height, std::vector<QuadraticBezier>& out) {
    auto ylo = std::min(q.p0.y, q.p2.y);
    auto yhi = std::max(q.p0.y, q.p2.y);
    auto eps = 1e-5f * height;

    auto bandStart = int(std::floor(ylo / height)) - 1;
    auto bandEnd = int(std::ceil(yhi / height)) + 1;

    std::vector<float> bounds;

    for (int k = bandStart; k <= bandEnd; ++k) {
        const float y = float(k) * height;
        if (y > (ylo + eps) && y < (yhi - eps))
            bounds.push_back(y);
    }

    if (q.p2.y < q.p0.y) std::reverse(bounds.begin(), bounds.end());

    for (float y : bounds) {
        const float t = std::clamp(SolveMonotonicBezierY(q, y), 0.f, 1.f);
        auto [a, b] = SplitQuadratic(q, t);
        a.p2.y = y;
        b.p0.y = y;
        ClampMonotone(a);
        ClampMonotone(b);
        out.push_back(a);
        q = b;
    }
    out.push_back(q);
}

void ApproximateCubicAsQuads(const CubicBezier& c, float tolerance, std::vector<QuadraticBezier>& out) {
    const glm::vec2 d = c.p3 - 3.0f * c.p2 + 3.0f * c.p1 - c.p0;
    const int n = std::clamp(int(std::ceil(std::cbrt(0.0481125f * glm::length(d) / tolerance))), 1, 128);
    CubicBezier rest = c;
    for (int i = 0; i < n; ++i) {
        CubicBezier piece = rest;
        if (i < n - 1) { auto [a, b] = SplitCubic(rest, 1.0f / float(n - i)); piece = a; rest = b; }
        out.push_back({piece.p0, (3.0f * (piece.p1 + piece.p2) - piece.p0 - piece.p3) * 0.25f, piece.p3});
    }
}


void QuadToMonotone(const QuadraticBezier& q, std::vector<QuadraticBezier>& out) {
    auto extremum = [](float a0, float a1, float a2) {
        const float den = a0 - 2.0f * a1 + a2;
        if (std::abs(den) < 1e-12f) return -1.0f;
        const float t = (a0 - a1) / den;
        return (t > 1e-6f && t < 1.0f - 1e-6f) ? t : -1.0f;
    };
    float ts[2]; int n = 0;
    if (float t = extremum(q.p0.x, q.p1.x, q.p2.x); t > 0) ts[n++] = t;
    if (float t = extremum(q.p0.y, q.p1.y, q.p2.y); t > 0) ts[n++] = t;
    if (n == 2 && ts[0] > ts[1]) std::swap(ts[0], ts[1]);
    if (n == 2 && ts[1] - ts[0] < 1e-6f) n = 1;
    QuadraticBezier rest = q; float prev = 0.0f;
    for (int i = 0; i < n; ++i) {
        auto [a, b] = SplitQuadratic(rest, (ts[i] - prev) / (1.0f - prev));
        ClampMonotone(a); out.push_back(a); rest = b; prev = ts[i];
    }
    ClampMonotone(rest); out.push_back(rest);
}

std::vector<QuadraticBezier> ToQuads(const sge::Path& path, float tolerance) {
    const auto& points = path.GetPoints();

    if (points.empty())
        return {};

    std::vector<QuadraticBezier> out;

    bool open = false;
    glm::vec2 current{ points[0] };
    glm::vec2 start{ points[0] };

    auto EmitLine = [&](glm::vec2 a, glm::vec2 b) {
        glm::vec2 d = glm::abs(b - a);
        if (d.x > 1e-6f || d.y > 1e-6f) {
            out.push_back(QuadraticBezier{ .p0=a, .p1=b, .p2=b });
        }
    };

    for (auto command : path.GetCommands()) {
        const size_t i = command.GetIndex();

        switch (command.GetType()) {
        case sge::Path::CommandType::MoveTo: {
            if (open)
                EmitLine(current, start);
            start = points[i];
            current = start;
            open = true;
        }
        break;
        case sge::Path::CommandType::LineTo: {
            auto p = points[i];
            EmitLine(current, p);
            current = p;
        }
        break;
        case sge::Path::CommandType::QuadraticBezierTo: {
            out.push_back({ .p0=current, .p1=points[i], .p2=points[i + 1] });
            current = points[i + 1];
        }
        break;
        case sge::Path::CommandType::CubicBezierTo: {
            ApproximateCubicAsQuads({.p0=current, .p1=points[i], .p2=points[i + 1], .p3=points[i + 2]}, tolerance, out);
            current = points[i + 2];
        }
        break;
        case sge::Path::CommandType::Close: {
            EmitLine(current, start);
            current = start;
            open = false;
        }
        break;
        }
    }

    if (open)
        EmitLine(current, start);

    return out;
}

} // anonymous namespace

sge::vector::ShapeData sge::vector::BuildFromPath(const sge::Path& path, const BuildOptions& opts) {
    std::vector<QuadraticBezier> mono;
    for (const QuadraticBezier& q : ToQuads(path, opts.tolerance))
        QuadToMonotone(q, mono);

    glm::vec2 lo{ FLT_MAX };
    glm::vec2 hi{ FLT_MIN };

    for (const QuadraticBezier& q : mono) {
        lo = glm::min(lo, q.p0);
        lo = glm::min(lo, q.p2);
        hi = glm::max(hi, q.p0);
        hi = glm::max(hi, q.p2);
    }

    ShapeData sd;
    sd.size = hi - lo;
    sd.origin = lo;
    if (sd.size.y <= 0.f)
        return sd;

    auto ToShape = [&](glm::vec2 p) {
        return glm::vec2(p.x - lo.x, p.y - lo.y);
    };
    for (QuadraticBezier& q : mono) {
        q = {.p0=ToShape(q.p0), .p1=ToShape(q.p1), .p2=ToShape(q.p2)};
        ClampMonotone(q);
    }

    const uint32_t nb = std::clamp(uint32_t(std::ceil(float(mono.size()) / opts.curvesPerBand)), 1u, opts.maxBands);
    sd.bandHeight = sd.size.y / nb;

    std::vector<std::vector<QuadraticBezier>> bands(nb);
    std::vector<QuadraticBezier> pieces;

    for (const QuadraticBezier& q : mono) {
        pieces.clear();
        SplitAtBandBoundaries(q, sd.bandHeight, pieces);
        for (const QuadraticBezier& p : pieces) {
            if (std::abs(p.p2.y - p.p0.y) <= 1e-5f * std::max(sd.size.x, sd.size.y))
                continue; // nearly horizontal: no area
            const int b = std::clamp(int(std::floor(0.5f * (p.p0.y + p.p2.y) / sd.bandHeight)), 0, int(nb) - 1);
            bands[b].push_back(p);
        }
    }
    for (uint32_t b = 0; b < nb; ++b) {
        sd.partitions.push_back({
            .curve_offset=uint32_t(sd.curves.size()),
            .curve_count=uint32_t(bands[b].size())
        });
        for (const QuadraticBezier& p : bands[b]) {
            sd.curves.push_back({
                .p0=p.p0,
                .p1=p.p1,
                .p2=p.p2
            });
        }
    }

    return sd;
}
