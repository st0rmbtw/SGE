#ifndef TYPES_RENDER_TARGET
#define TYPES_RENDER_TARGET

#include <LLGL/RenderTargetFlags.h>
#include <LLGL/RenderTarget.h>

struct RenderTarget {
    LLGL::TextureDescriptor texture_descriptor;
    LLGL::RenderTarget* internal;
    LLGL::Texture* texture;
};

#endif