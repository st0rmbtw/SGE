#ifndef SGE_TYPES_VECTOR_SHAPE_HPP_
#define SGE_TYPES_VECTOR_SHAPE_HPP_

#include <utility>
#include <span>

#include <SGE/assert.hpp>
#include <SGE/renderer/resource.hpp>
#include <SGE/types/color.hpp>
#include <SGE/types/path.hpp>

#include <glm/vec2.hpp>

#include <LLGL/Buffer.h>

namespace sge {

struct VectorShapeLayer {
    sge::Path path;
    sge::LinearRgba color{ sge::color::WHITE };
    std::optional<glm::vec2> customSize{};
};

enum class VectorShapeLayerId : uint32_t {
    INVALID = static_cast<uint32_t>(-1),
    ALL = static_cast<uint32_t>(-2),
};

struct VectorShapeDesc {
    std::span<const VectorShapeLayer> layers;
    float tolerance{ 0.01f };
    uint32_t curvesPerBand{ 4 };
    uint32_t maxBands{ 64 };
    bool flipY{ false };
};

struct GpuVectorShapeLayer {
    glm::vec2 size;
    uint32_t partitionOffset;
    uint32_t partitionCount;
    float bandHeight;
};

struct GpuVectorShapeDesc {
    sge::Unique<LLGL::Buffer> curvesBuffer;
    sge::Unique<LLGL::Buffer> partitionsBuffer;
    std::vector<GpuVectorShapeLayer> layers;
};

// Owns layers metadata and GPU buffers with shape data
class GpuVectorShape {
public:
    explicit GpuVectorShape(GpuVectorShapeDesc desc) :
        m_curves_buffer{ std::move(desc.curvesBuffer) },
        m_partitions_buffer{ std::move(desc.partitionsBuffer) },
        m_layers { std::move(desc.layers) }
    {}

    [[nodiscard]]
    LLGL::Buffer& GetCurvesBuffer() const {
        return *m_curves_buffer;
    }

    [[nodiscard]]
    LLGL::Buffer& GetPartitionsBuffer() const {
        return *m_partitions_buffer;
    }

    [[nodiscard]]
    std::span<const GpuVectorShapeLayer> GetLayers() const {
        return m_layers;
    }

    [[nodiscard]]
    const GpuVectorShapeLayer& GetLayer(VectorShapeLayerId id) const {
        SGE_ASSERT_M(id != VectorShapeLayerId::INVALID, "id is not valid!");
        SGE_ASSERT_M(id != VectorShapeLayerId::ALL, "id is not valid!");

        const auto index = std::to_underlying(id);
        SGE_ASSERT_M(index < m_layers.size(), "no layer with id `{}`.", index);
        return m_layers[index];
    }

private:
    sge::Unique<LLGL::Buffer> m_curves_buffer;
    sge::Unique<LLGL::Buffer> m_partitions_buffer;
    std::vector<GpuVectorShapeLayer> m_layers;
};

}

#endif // SGE_TYPES_VECTOR_SHAPES_HPP_
