#ifndef _SGE_TYPES_SAMPLER_HPP_
#define _SGE_TYPES_SAMPLER_HPP_

#include <LLGL/Sampler.h>
#include <LLGL/SamplerFlags.h>

#include "../defines.hpp"

_SGE_BEGIN

class Sampler {
public:
    Sampler() = default;

    explicit Sampler(LLGL::Sampler* internal, LLGL::SamplerDescriptor descriptor) noexcept :
        m_internal(internal),
        m_descriptor(descriptor) {}

    [[nodiscard]]
    inline const LLGL::SamplerDescriptor& descriptor() const noexcept {
        return m_descriptor;
    }

    [[nodiscard]]
    inline LLGL::Sampler* internal() const noexcept {
        return m_internal;
    }

    [[nodiscard]]
    inline operator LLGL::Sampler&() const noexcept {
        return *m_internal;
    }

private:
    LLGL::Sampler* m_internal = nullptr;
    LLGL::SamplerDescriptor m_descriptor;
};

_SGE_END

#endif