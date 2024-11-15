#ifndef ASSETS_HPP
#define ASSETS_HPP

#pragma once

#include <LLGL/Sampler.h>
#include <LLGL/Utils/VertexFormat.h>
#include <glm/glm.hpp>

#include "types/texture_atlas.hpp"
#include "types/texture.hpp"
#include "types/shader_pipeline.hpp"
#include "types/font.hpp"

enum class TextureAsset : uint8_t {
    Stub = 0,
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

struct ShaderDef {
    std::string name;
    std::string value;
    
    ShaderDef(std::string name, std::string value) :
        name(std::move(name)),
        value(std::move(value)) {}
};

constexpr uint32_t PARTICLES_ATLAS_COLUMNS = 100;

namespace Assets {
    bool Load();
    bool LoadShaders(const std::vector<ShaderDef>& shader_defs = {});
    bool LoadFonts();
    bool InitSamplers();
    void InitVertexFormats();

    void DestroyTextures();
    void DestroyShaders();
    void DestroySamplers();
    void DestroyFonts();

    const Texture& GetTexture(TextureAsset key);
    const TextureAtlas& GetTextureAtlas(TextureAsset key);
    const Font& GetFont(FontAsset key);
    const Texture& GetItemTexture(size_t index);
    const ShaderPipeline& GetShader(ShaderAsset key);
    LLGL::Shader* GetComputeShader(ComputeShaderAsset key);
    LLGL::Sampler& GetSampler(size_t index);
    const LLGL::VertexFormat& GetVertexFormat(VertexFormatAsset key);
};

#endif