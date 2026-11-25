#ifndef ENGINE_RENDERER_VECTOR_CONVERT_HPP_
#define ENGINE_RENDERER_VECTOR_CONVERT_HPP_

#include <vector>
#include <glm/vec2.hpp>

namespace sge {
class Path;

namespace vector {

struct Curve {
    glm::vec2 p0, p1, p2;
};

struct Partition {
    uint32_t curveOffset = 0, curveCount = 0;
};

struct ShapeData {
    std::vector<Curve> curves;
    std::vector<Partition> partitions;
    glm::vec2 origin{ 0.f };
    glm::vec2 size{ 0.f };
    float bandHeight{ 1.f };
};

struct BuildOptions {
    float tolerance{ 0.01f };
    uint32_t curvesPerBand{ 4 };
    uint32_t maxBands{ 64 };
    bool flipY{ false };
};

ShapeData BuildFromPath(const sge::Path& path, const BuildOptions& opts);

} // vector

} // sge

#endif // ENGINE_RENDERER_VECTOR_CONVERT_HPP_
