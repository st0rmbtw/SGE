#ifndef SGE_TYPES_VECTOR_SHAPES_HPP_
#define SGE_TYPES_VECTOR_SHAPES_HPP_

#include <SGE/renderer/resource.hpp>
#include <SGE/types/path.hpp>
#include <LLGL/Buffer.h>
#include <glm/vec2.hpp>

namespace sge {

class VectorShape {
public:
    explicit VectorShape(
        sge::Ref<LLGL::Buffer> curvesBuffer,
        sge::Ref<LLGL::Buffer> partitionsBuffer,
        glm::vec2 size,
        float bandHeight,
        uint32_t partitionCount
    ) : m_curves_buffer{ std::move(curvesBuffer) },
        m_partitions_buffer{ std::move(partitionsBuffer) },
        m_size{ size },
        m_band_height{ bandHeight },
        m_partition_count{ partitionCount }
    {}

    [[nodiscard]]
    const sge::Ref<LLGL::Buffer>& GetCurvesBuffer() const {
        return m_curves_buffer;
    }

    [[nodiscard]]
    const sge::Ref<LLGL::Buffer>& GetPartitionsBuffer() const {
        return m_partitions_buffer;
    }

    [[nodiscard]]
    glm::vec2 GetSize() const {
        return m_size;
    }

    [[nodiscard]]
    float GetBandHeight() const {
        return m_band_height;
    }

    [[nodiscard]]
    uint32_t GetPartitionCount() const {
        return m_partition_count;
    }

private:
    sge::Ref<LLGL::Buffer> m_curves_buffer;
    sge::Ref<LLGL::Buffer> m_partitions_buffer;
    glm::vec2 m_size{ 0.f };
    float m_band_height{ 0.f };
    uint32_t m_partition_count{ 0 };
};

}

#endif // SGE_TYPES_VECTOR_SHAPES_HPP_
