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
    bool LoadTextures(sge::Renderer& renderer);
    bool LoadFonts(sge::Renderer& renderer);

    void DestroyTextures(sge::Renderer& renderer);
    void DestroySamplers(sge::Renderer& renderer);
    void DestroyFonts();

    const sge::Texture& GetTexture(TextureAsset key);
    const sge::TextureAtlas& GetTextureAtlas(TextureAsset key);
    const sge::Font& GetFont(FontAsset key);
    const sge::Sampler& GetSampler(size_t index);
};

#endif