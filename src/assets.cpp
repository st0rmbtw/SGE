#include "assets.hpp"

#include <array>

#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/Shader.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/VertexAttribute.h>
#include <STB/stb_image.h>
#include <ft2build.h>
#include <freetype/freetype.h>

#include "LLGL/Format.h"
#include "LLGL/TextureFlags.h"
#include "engine/renderer/renderer.hpp"
#include "engine/types/texture.hpp"
#include "engine/log.hpp"

#include "engine/utils.hpp"

using namespace sge::types;
using namespace sge::renderer;

struct AssetTextureAtlas {
    uint32_t rows;
    uint32_t columns;
    glm::uvec2 tile_size;
    glm::uvec2 padding = glm::uvec2(0);
    glm::uvec2 offset = glm::uvec2(0);

    AssetTextureAtlas(uint32_t columns, uint32_t rows, glm::uvec2 tile_size, glm::uvec2 padding = glm::uvec2(0), glm::uvec2 offset = glm::uvec2(0)) :
        rows(rows),
        columns(columns),
        tile_size(tile_size),
        padding(padding),
        offset(offset) {}
};

struct AssetTexture {
    std::string path;
    int sampler;

    explicit AssetTexture(std::string path, int sampler = TextureSampler::Nearest) :
        path(std::move(path)),
        sampler(sampler) {}
};


static const std::pair<TextureAsset, AssetTexture> TEXTURE_ASSETS[] = {
    {TextureAsset::DesktopBackground, AssetTexture("assets/images/monterey_wallpaper.jpg", TextureSampler::Linear)},
    {TextureAsset::IconFinder, AssetTexture("assets/images/finder.png", TextureSampler::Linear)},
    {TextureAsset::IconAppStore, AssetTexture("assets/images/appstore.png", TextureSampler::Linear)},
    {TextureAsset::IconBin, AssetTexture("assets/images/bin.png", TextureSampler::Linear)},
    {TextureAsset::IconBooks, AssetTexture("assets/images/books.png", TextureSampler::Linear)},
    {TextureAsset::IconCalculator, AssetTexture("assets/images/calculator.png", TextureSampler::Linear)},
    {TextureAsset::IconCalendar, AssetTexture("assets/images/calendar.png", TextureSampler::Linear)},
    {TextureAsset::IconContacts, AssetTexture("assets/images/contacts.png", TextureSampler::Linear)},
    {TextureAsset::IconLaunchpad, AssetTexture("assets/images/launchpad.png", TextureSampler::Linear)},
    {TextureAsset::IconMail, AssetTexture("assets/images/mail.png", TextureSampler::Linear)},
    {TextureAsset::IconMaps, AssetTexture("assets/images/maps.png", TextureSampler::Linear)},
    {TextureAsset::IconMessage, AssetTexture("assets/images/message.png", TextureSampler::Linear)},
    {TextureAsset::IconMusic, AssetTexture("assets/images/music.png", TextureSampler::Linear)},
    {TextureAsset::IconNotes, AssetTexture("assets/images/notes.png", TextureSampler::Linear)},
    {TextureAsset::IconPhotos, AssetTexture("assets/images/photos.png", TextureSampler::Linear)},
    {TextureAsset::IconPreferences, AssetTexture("assets/images/preferences.png", TextureSampler::Linear)},
    {TextureAsset::IconReminders, AssetTexture("assets/images/reminders.png", TextureSampler::Linear)},
    {TextureAsset::IconTerminal, AssetTexture("assets/images/terminal.png", TextureSampler::Linear)},
};

static const std::pair<TextureAsset, AssetTextureAtlas> TEXTURE_ATLAS_ASSETS[] = {
    
};

static const std::pair<FontAsset, const char*> FONT_ASSETS[] = {

};

static struct AssetsState {
    std::unordered_map<TextureAsset, Texture> textures;
    std::unordered_map<TextureAsset, TextureAtlas> textures_atlases;
    std::unordered_map<ComputeShaderAsset, LLGL::Shader*> compute_shaders;
    std::unordered_map<FontAsset, Font> fonts;
    std::vector<Sampler> samplers;
    uint32_t texture_index = 0;
} state;

static bool load_font(Renderer& renderer, FT_Library ft, const std::string& path, Font& font);
static bool load_texture(Renderer& renderer, const char* path, int sampler, Texture* texture);

template <size_t T>
static Texture load_texture_array(const std::array<std::pair<uint32_t, const char*>, T>& assets, int sampler, bool generate_mip_maps = false);

static void InitSamplers(Renderer& renderer) {
    state.samplers.resize(4);
    {
        LLGL::SamplerDescriptor sampler_desc;
        sampler_desc.addressModeU = LLGL::SamplerAddressMode::Clamp;
        sampler_desc.addressModeV = LLGL::SamplerAddressMode::Clamp;
        sampler_desc.addressModeW = LLGL::SamplerAddressMode::Clamp;
        sampler_desc.magFilter = LLGL::SamplerFilter::Linear;
        sampler_desc.minFilter = LLGL::SamplerFilter::Linear;
        sampler_desc.mipMapFilter = LLGL::SamplerFilter::Linear;
        sampler_desc.minLOD = 0.0f;
        sampler_desc.maxLOD = 1.0f;
        sampler_desc.mipMapEnabled = false;
        sampler_desc.maxAnisotropy = 1;

        state.samplers[TextureSampler::Linear] = renderer.CreateSampler(sampler_desc);
    }
    {
        LLGL::SamplerDescriptor sampler_desc;
        sampler_desc.addressModeU = LLGL::SamplerAddressMode::Clamp;
        sampler_desc.addressModeV = LLGL::SamplerAddressMode::Clamp;
        sampler_desc.addressModeW = LLGL::SamplerAddressMode::Clamp;
        sampler_desc.magFilter = LLGL::SamplerFilter::Linear;
        sampler_desc.minFilter = LLGL::SamplerFilter::Linear;
        sampler_desc.mipMapFilter = LLGL::SamplerFilter::Linear;
        sampler_desc.minLOD = 0.0f;
        sampler_desc.maxLOD = 100.0f;
        sampler_desc.mipMapEnabled = true;
        sampler_desc.maxAnisotropy = 1;

        state.samplers[TextureSampler::LinearMips] = renderer.CreateSampler(sampler_desc);
    }
    {
        LLGL::SamplerDescriptor sampler_desc;
        sampler_desc.addressModeU = LLGL::SamplerAddressMode::Clamp;
        sampler_desc.addressModeV = LLGL::SamplerAddressMode::Clamp;
        sampler_desc.addressModeW = LLGL::SamplerAddressMode::Clamp;
        sampler_desc.magFilter = LLGL::SamplerFilter::Nearest;
        sampler_desc.minFilter = LLGL::SamplerFilter::Nearest;
        sampler_desc.mipMapFilter = LLGL::SamplerFilter::Linear;
        sampler_desc.minLOD = 0.0f;
        sampler_desc.maxLOD = 1.0f;
        sampler_desc.mipMapEnabled = false;
        sampler_desc.maxAnisotropy = 1;

        state.samplers[TextureSampler::Nearest] = renderer.CreateSampler(sampler_desc);
    }
    {
        LLGL::SamplerDescriptor sampler_desc;
        sampler_desc.addressModeU = LLGL::SamplerAddressMode::Clamp;
        sampler_desc.addressModeV = LLGL::SamplerAddressMode::Clamp;
        sampler_desc.addressModeW = LLGL::SamplerAddressMode::Clamp;
        sampler_desc.magFilter = LLGL::SamplerFilter::Nearest;
        sampler_desc.minFilter = LLGL::SamplerFilter::Nearest;
        sampler_desc.mipMapFilter = LLGL::SamplerFilter::Linear;
        sampler_desc.minLOD = 0.0f;
        sampler_desc.maxLOD = 100.0f;
        sampler_desc.mipMapEnabled = true;
        sampler_desc.maxAnisotropy = 1;

        state.samplers[TextureSampler::NearestMips] = renderer.CreateSampler(sampler_desc);
    }
}

bool Assets::LoadTextures(Renderer& renderer) {
    InitSamplers(renderer);

    const uint8_t data[] = { 0xFF, 0xFF, 0xFF, 0xFF };
    state.textures[TextureAsset::Stub] = renderer.CreateTexture(LLGL::TextureType::Texture2D, LLGL::ImageFormat::RGBA, 1, 1, 1, GetSampler(TextureSampler::Nearest), data);

    for (const auto& [key, asset] : TEXTURE_ASSETS) {
        Texture texture;
        if (!load_texture(renderer, asset.path.c_str(), asset.sampler, &texture)) {
            return false;
        }
        state.textures[key] = texture;
    }

    for (const auto& [key, asset] : TEXTURE_ATLAS_ASSETS) {
        state.textures_atlases[key] = TextureAtlas::from_grid(Assets::GetTexture(key), asset.tile_size, asset.columns, asset.rows, asset.padding, asset.offset);
    }

    return true;
}

bool Assets::LoadFonts(Renderer& renderer) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        LOG_ERROR("Couldn't init FreeType library.");
        return false;
    }

    for (const auto& [key, path] : FONT_ASSETS) {
        if (!sge::utils::FileExists(path)) {
            LOG_ERROR("Failed to find font '%s'",  path);
            return false;
        }

        Font font;
        if (!load_font(renderer, ft, path, font)) return false;

        state.fonts[key] = font;
    }

    FT_Done_FreeType(ft);

    return true;
}

void Assets::DestroyTextures(Renderer& renderer) {
    for (auto& entry : state.textures) {
        renderer.Context()->Release(entry.second);
    }
}

void Assets::DestroySamplers(Renderer& renderer) {
    for (auto& sampler : state.samplers) {
        renderer.Context()->Release(sampler);
    }
}

const Texture& Assets::GetTexture(TextureAsset key) {
    const auto entry = std::as_const(state.textures).find(key);
    ASSERT(entry != state.textures.cend(), "Texture not found: %u", static_cast<uint32_t>(key));
    return entry->second;
}

const TextureAtlas& Assets::GetTextureAtlas(TextureAsset key) {
    const auto entry = std::as_const(state.textures_atlases).find(key);
    ASSERT(entry != state.textures_atlases.cend(), "TextureAtlas not found: %u", static_cast<uint32_t>(key));
    return entry->second;
}

const Font& Assets::GetFont(FontAsset key) {
    const auto entry = std::as_const(state.fonts).find(key);
    ASSERT(entry != state.fonts.cend(), "Font not found: %u", static_cast<uint32_t>(key));
    return entry->second;
}

LLGL::Shader* Assets::GetComputeShader(ComputeShaderAsset key) {
    const auto entry = std::as_const(state.compute_shaders).find(key);
    ASSERT(entry != state.compute_shaders.cend(), "ComputeShader not found: %u", static_cast<uint32_t>(key));
    return entry->second;
}

const Sampler& Assets::GetSampler(size_t index) {
    ASSERT(index < state.samplers.size(), "Index is out of bounds: %zu", index);
    return state.samplers[index];
}

static bool load_texture(Renderer& renderer, const char* path, int sampler, Texture* texture) {
    int width, height, components;

    uint8_t* data = stbi_load(path, &width, &height, &components, 4);
    if (data == nullptr) {
        LOG_ERROR("Couldn't load asset: %s", path);
        return false;
    }

    *texture = renderer.CreateTexture(LLGL::TextureType::Texture2D, LLGL::ImageFormat::RGBA, width, height, 1, Assets::GetSampler(sampler), data);

    stbi_image_free(data);

    return true;
}

template <size_t T>
static Texture load_texture_array(const std::array<std::pair<uint32_t, const char*>, T>& assets, int sampler, bool generate_mip_maps) {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t layers_count = 0;

    struct Layer {
        uint8_t* data;
        size_t size;
    };

    std::unordered_map<uint32_t, Layer> layer_to_data_map;
    layer_to_data_map.reserve(assets.size());

    for (const auto& [index, path] : assets) {
        layers_count = glm::max(layers_count, index);

        int w, h;
        uint8_t* layer_data = stbi_load(path, &w, &h, nullptr, 4);
        if (layer_data == nullptr) {
            LOG_ERROR("Couldn't load asset: %s", path);
            continue;
        }

        layer_to_data_map[index] = Layer {
            .data = layer_data,
            .size = static_cast<size_t>(w * h * 4)
        };

        width = glm::max(width, static_cast<uint32_t>(w));
        height = glm::max(height, static_cast<uint32_t>(h));
    }
    layers_count += 1;

    const size_t data_size = width * height * layers_count * 4;
    uint8_t* image_data = new uint8_t[data_size];

    for (const auto& [layer, layer_data] : layer_to_data_map) {
        memcpy(&image_data[width * height * 4 * layer], layer_data.data, layer_data.size);
        stbi_image_free(layer_data.data);
    }
    
    return create_texture_array(width, height, layers_count, 4, sampler, image_data, data_size, generate_mip_maps);
}

static bool load_font(Renderer& renderer, FT_Library ft, const std::string& path, Font& font) {
    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face)) {
        LOG_ERROR("Failed to load font: %s", path.c_str());
        return false;
    }

    static constexpr uint32_t FONT_SIZE = 22;
    static constexpr uint32_t PADDING = 2;

    FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);

    static constexpr uint32_t texture_width = 512;
    static constexpr uint32_t texture_height = 512;

    const glm::vec2 texture_size = glm::vec2(texture_width, texture_height);

    uint32_t row = 0;
    uint32_t col = PADDING;

    uint8_t* texture_data = new uint8_t[texture_width * texture_height];
    memset(texture_data, 0, texture_width * texture_height * sizeof(uint8_t));

    for (unsigned char c = 32; c < 127; ++c) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            LOG_ERROR("Failed to load glyph '%c'", c);
            continue;
        }

        if (col + face->glyph->bitmap.width + PADDING >= texture_width) {
            col = PADDING;
            row += FONT_SIZE;
        }

        for (uint32_t y = 0; y < face->glyph->bitmap.rows; ++y) {
            for (uint32_t x = 0; x < face->glyph->bitmap.width; ++x) {
                texture_data[(row + y) * texture_width + col + x] = face->glyph->bitmap.buffer[y * face->glyph->bitmap.width + x];
            }
        }

        const glm::ivec2 size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
        
        Glyph glyph = {
            .size = size,
            .tex_size = glm::vec2(size) / texture_size,
            .bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            .advance = face->glyph->advance.x,
            .texture_coords = glm::vec2(col, row) / texture_size
        };

        font.glyphs[c] = glyph;

        col += face->glyph->bitmap.width + PADDING;
    }

    font.texture = renderer.CreateTexture(LLGL::TextureType::Texture2D, LLGL::ImageFormat::R, texture_width, texture_height, 1, Assets::GetSampler(TextureSampler::Linear), texture_data);
    font.font_size = FONT_SIZE;

    FT_Done_Face(face);

    return true;
}