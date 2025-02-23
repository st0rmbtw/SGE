#ifndef ASSETS_HPP
#define ASSETS_HPP

#pragma once

#include <LLGL/Sampler.h>
#include <LLGL/Utils/VertexFormat.h>
#include <glm/glm.hpp>

#include "engine/renderer/renderer.hpp"
#include "engine/types/texture_atlas.hpp"
#include "engine/types/texture.hpp"
#include "engine/types/font.hpp"

enum class TextureAsset : uint8_t {
    Stub = 0,
    IconFinder,
    IconAppStore,
    IconBin,
    IconBooks,
    IconCalculator,
    IconCalendar,
    IconContacts,
    IconLaunchpad,
    IconMail,
    IconMaps,
    IconMessage,
    IconMusic,
    IconNotes,
    IconPhotos,
    IconPreferences,
    IconReminders,
    IconTerminal,
    DesktopBackground
};

enum class ShaderAsset : uint8_t {
    SpriteShader = 0,
    FontShader,
    ShapeShader
};

enum class ComputeShaderAsset : uint8_t {

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
    LLGL::Shader* GetComputeShader(ComputeShaderAsset key);
    const sge::types::Sampler& GetSampler(size_t index);
};

#endif