#ifndef _SGE_TYPES_SAMPLER_HPP_
#define _SGE_TYPES_SAMPLER_HPP_

#include <utility>
#include <SGE/renderer/resource.hpp>
#include <LLGL/Sampler.h>
#include <LLGL/SamplerFlags.h>

namespace sge {

class Sampler : public RefCounted {
public:
    Sampler() = default;

    explicit Sampler(Unique<LLGL::Sampler> internal, LLGL::SamplerDescriptor descriptor) noexcept :
        m_descriptor(descriptor),
        m_internal(std::move(internal)) {}

    [[nodiscard]]
    inline const LLGL::SamplerDescriptor& descriptor() const noexcept {
        return m_descriptor;
    }

    [[nodiscard]]
    inline const Unique<LLGL::Sampler>& internal() const noexcept {
        return m_internal;
    }

    [[nodiscard]]
    inline operator LLGL::Sampler&() const noexcept {
        return *m_internal;
    }

private:
    LLGL::SamplerDescriptor m_descriptor;
    Unique<LLGL::Sampler> m_internal = nullptr;
};

}

#endif