#ifndef ASSETS_HPP
#define ASSETS_HPP

#pragma once

#include <LLGL/Sampler.h>
#include <LLGL/Utils/VertexFormat.h>
#include <glm/glm.hpp>

#include <SGE/renderer/renderer.hpp>
#include <SGE/types/texture_atlas.hpp>
#include <SGE/types/texture.hpp>
#include <SGE/types/font.hpp>

enum class TextureAsset : uint8_t {
    Stub = 0,
};

enum class FontAsset : uint8_t {

};

enum class VertexFormatAsset : uint8_t {
    SpriteVertex = 0,
    SpriteInstance,
    FontVertex,
    FontInstance,
    ShapeVertex,
    ShapeInstance
};

constexpr uint32_t PARTICLES_ATLAS_COLUMNS = 100;

namespace Assets {
    bool LoadTextures(sge::renderer::Renderer& renderer);
    bool LoadFonts(sge::renderer::Renderer& renderer);

    void DestroyTextures(sge::renderer::Renderer& renderer);
    void DestroySamplers(sge::renderer::Renderer& renderer);
    void DestroyFonts();

    const sge::types::Texture& GetTexture(TextureAsset key);
    const sge::types::TextureAtlas& GetTextureAtlas(TextureAsset key);
    const sge::types::Font& GetFont(FontAsset key);
    const sge::types::Sampler& GetSampler(size_t index);
};

#endif