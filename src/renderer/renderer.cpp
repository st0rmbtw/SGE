#include "renderer.hpp"

#include <memory>
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/CommandBufferFlags.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/RendererConfiguration.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../assets.hpp"
#include "../log.hpp"

#include "LLGL/RenderPassFlags.h"
#include "LLGL/RenderTarget.h"
#include "types.hpp"
#include "utils.hpp"
#include "batch.hpp"

static struct RendererState {
    RenderBatchSprite sprite_batch;
    RenderBatchGlyph glyph_batch;
    RenderBatchShape shape_batch;

    math::Rect ui_frustum;
    math::Rect camera_frustum;

    LLGL::RenderSystemPtr context = nullptr;
    std::shared_ptr<CustomSurface> surface = nullptr;
    
    LLGL::SwapChain* swap_chain = nullptr;
    LLGL::CommandBuffer* command_buffer = nullptr;
    LLGL::CommandQueue* command_queue = nullptr;
#if DEBUG
    LLGL::RenderingDebugger* debugger = nullptr;
#endif

    LLGL::Buffer* constant_buffer = nullptr;

    LLGL::RenderPass* load_render_pass = nullptr;
    
    uint32_t main_depth_index = 0;
    uint32_t ui_depth_index = 0;
    uint32_t prev_max_depth = 0;

    RenderBackend backend;
} state;

const LLGL::RenderSystemPtr& Renderer::Context() { return state.context; }
LLGL::SwapChain* Renderer::SwapChain() { return state.swap_chain; }
LLGL::CommandBuffer* Renderer::CommandBuffer() { return state.command_buffer; }
LLGL::CommandQueue* Renderer::CommandQueue() { return state.command_queue; }
const std::shared_ptr<CustomSurface>& Renderer::Surface() { return state.surface; }
LLGL::Buffer* Renderer::GlobalUniformBuffer() { return state.constant_buffer; }
RenderBackend Renderer::Backend() { return state.backend; }
uint32_t Renderer::GetMainDepthIndex() { return state.main_depth_index; };
uint32_t Renderer::GetUiDepthIndex() { return state.ui_depth_index; };

bool Renderer::InitEngine(RenderBackend backend) {
    LLGL::Report report;

    void* configPtr = nullptr;
    size_t configSize = 0;

    if (backend.IsOpenGL()) {
        LLGL::RendererConfigurationOpenGL config;
        config.majorVersion = 4;
        config.minorVersion = 3;
        config.contextProfile = LLGL::OpenGLContextProfile::CoreProfile;
        configPtr = &config;
        configSize = sizeof(config);
    }

    LLGL::RenderSystemDescriptor rendererDesc;
    rendererDesc.moduleName = backend.ToString();
    rendererDesc.rendererConfig = configPtr;
    rendererDesc.rendererConfigSize = configSize;

#if DEBUG
    state.debugger = new LLGL::RenderingDebugger();
    rendererDesc.flags    = LLGL::RenderSystemFlags::DebugDevice;
    rendererDesc.debugger = state.debugger;
#endif

    state.context = LLGL::RenderSystem::Load(rendererDesc, &report);
    state.backend = backend;

    if (report.HasErrors()) {
        LOG_ERROR("%s", report.GetText());
        return false;
    }

    const auto& info = state.context->GetRendererInfo();

    LOG_INFO("Renderer:             %s", info.rendererName.c_str());
    LOG_INFO("Device:               %s", info.deviceName.c_str());
    LOG_INFO("Vendor:               %s", info.vendorName.c_str());
    LOG_INFO("Shading Language:     %s", info.shadingLanguageName.c_str());

    LOG_INFO("Extensions:");
    for (const std::string& extension : info.extensionNames) {
        LOG_INFO("  %s", extension.c_str());
    }

    return true;
}

bool Renderer::Init(GLFWwindow* window, const LLGL::Extent2D& resolution, bool vsync, bool fullscreen) {
    const LLGL::RenderSystemPtr& context = state.context;

    state.surface = std::make_shared<CustomSurface>(window, resolution);

    LLGL::SwapChainDescriptor swapChainDesc;
    swapChainDesc.resolution = resolution;
    swapChainDesc.fullscreen = fullscreen;

    state.swap_chain = context->CreateSwapChain(swapChainDesc, state.surface);
    state.swap_chain->SetVsyncInterval(vsync);

    state.command_buffer = context->CreateCommandBuffer();
    state.command_queue = context->GetCommandQueue();

    state.constant_buffer = CreateConstantBuffer(sizeof(ProjectionsUniform));

    LLGL::RenderPassDescriptor load_render_pass;
    load_render_pass.colorAttachments[0].loadOp = LLGL::AttachmentLoadOp::Load;
    load_render_pass.colorAttachments[0].storeOp = LLGL::AttachmentStoreOp::Store;
    load_render_pass.depthAttachment.loadOp = LLGL::AttachmentLoadOp::Load;
    load_render_pass.depthAttachment.storeOp = LLGL::AttachmentStoreOp::Store;
    load_render_pass.stencilAttachment.loadOp = LLGL::AttachmentLoadOp::Load;
    load_render_pass.stencilAttachment.storeOp = LLGL::AttachmentStoreOp::Store;

    state.load_render_pass = context->CreateRenderPass(load_render_pass);

    state.sprite_batch.init();
    state.glyph_batch.init();
    state.shape_batch.init();

    return true;
}

void Renderer::Begin(const Camera& camera) {
    auto* const commands = Renderer::CommandBuffer();
    auto* const swap_chain = Renderer::SwapChain();

    const math::Rect camera_frustum = math::Rect::from_corners(
        camera.position() + camera.get_projection_area().min,
        camera.position() + camera.get_projection_area().max
    );
    const math::Rect ui_frustum = math::Rect::from_corners(glm::vec2(0.0), camera.viewport());

    state.camera_frustum = camera_frustum;
    state.ui_frustum = ui_frustum;
    state.main_depth_index = 0;
    state.ui_depth_index = 0;

    commands->Begin();
    commands->SetViewport(swap_chain->GetResolution());

    state.sprite_batch.begin();
    state.glyph_batch.begin();
    state.shape_batch.begin();
}

void Renderer::Render(const Camera& camera, LLGL::ClearValue clear) {
    auto* const commands = Renderer::CommandBuffer();
    auto* const queue = state.command_queue;
    auto* const swap_chain = Renderer::SwapChain();

    const uint32_t max_depth = state.main_depth_index + state.ui_depth_index;

    if (camera.mutated() || state.prev_max_depth != max_depth) {
        auto projections_uniform = ProjectionsUniform {
            .screen_projection_matrix = camera.get_screen_projection_matrix(),
            .view_projection_matrix = camera.get_view_projection_matrix(),
            .transform_matrix = camera.get_transform_matrix(),
            .camera_position = camera.position(),
            .window_size = camera.viewport(),
            .max_depth = static_cast<float>(max_depth)
        };

        commands->UpdateBuffer(*state.constant_buffer, 0, &projections_uniform, sizeof(projections_uniform));
    }

    commands->BeginRenderPass(*Renderer::SwapChain());
        commands->Clear(LLGL::ClearFlags::ColorDepth, clear);

        state.sprite_batch.render();
        state.glyph_batch.render();
        state.shape_batch.render();
    commands->EndRenderPass();

    commands->End();
    queue->Submit(*commands);

    swap_chain->Present();

    state.prev_max_depth = max_depth;
}

#if DEBUG
void Renderer::PrintDebugInfo() {
    LLGL::FrameProfile profile;
    state.debugger->FlushProfile(&profile);

    LOG_DEBUG("Draw commands count: %u", profile.commandBufferRecord.drawCommands);
}
#endif

inline void add_sprite_to_batch(const Sprite& sprite, RenderLayer layer, bool is_ui, int depth, LLGL::RenderTarget* render_target) {
    glm::vec4 uv_offset_scale = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

    if (sprite.flip_x()) {
        uv_offset_scale.x += uv_offset_scale.z;
        uv_offset_scale.z *= -1.0f;
    }

    if (sprite.flip_y()) {
        uv_offset_scale.y += uv_offset_scale.w;
        uv_offset_scale.w *= -1.0f;
    }

    state.sprite_batch.draw_sprite(sprite, uv_offset_scale, sprite.texture(), layer, is_ui, depth, render_target);
}

inline void add_atlas_sprite_to_batch(const TextureAtlasSprite& sprite, RenderLayer layer, bool is_ui, int depth) {
    const math::Rect& rect = sprite.atlas().get_rect(sprite.index());

    glm::vec4 uv_offset_scale = glm::vec4(
        rect.min.x / sprite.atlas().texture().size.x,
        rect.min.y / sprite.atlas().texture().size.y,
        rect.size().x / sprite.atlas().texture().size.x,
        rect.size().y / sprite.atlas().texture().size.y
    );

    if (sprite.flip_x()) {
        uv_offset_scale.x += uv_offset_scale.z;
        uv_offset_scale.z *= -1.0f;
    }

    if (sprite.flip_y()) {
        uv_offset_scale.y += uv_offset_scale.w;
        uv_offset_scale.w *= -1.0f;
    }

    state.sprite_batch.draw_sprite(sprite, uv_offset_scale, sprite.atlas().texture(), layer, is_ui, depth);
}

void Renderer::DrawSprite(const Sprite& sprite, RenderLayer render_layer, int depth, LLGL::RenderTarget* render_target) {
    const math::Rect aabb = sprite.calculate_aabb();

    if (!state.camera_frustum.intersects(aabb)) return;

    add_sprite_to_batch(sprite, render_layer, false, depth, render_target);
}

void Renderer::DrawSpriteUI(const Sprite& sprite, int depth) {
    const math::Rect aabb = sprite.calculate_aabb();
    if (!state.ui_frustum.intersects(aabb)) return;

    add_sprite_to_batch(sprite, RenderLayer::Main, true, depth, nullptr);
}

void Renderer::DrawAtlasSprite(const TextureAtlasSprite& sprite, RenderLayer render_layer, int depth) {
    const math::Rect aabb = sprite.calculate_aabb();

    if (!state.camera_frustum.intersects(aabb)) return;

    add_atlas_sprite_to_batch(sprite, render_layer, false, depth);
}

void Renderer::DrawAtlasSpriteUI(const TextureAtlasSprite& sprite, int depth) {
    const math::Rect aabb = sprite.calculate_aabb();
    if (!state.ui_frustum.intersects(aabb)) return;

    add_atlas_sprite_to_batch(sprite, RenderLayer::Main, true, depth);
}

void Renderer::DrawShape(Shape::Type shape, glm::vec2 position, glm::vec2 size, const glm::vec4& color, const glm::vec4& border_color, float border_thickness, float border_radius, Anchor anchor, bool is_ui, int depth) {
    const math::Rect aabb = math::Rect::from_top_left(position - anchor.to_vec2() * size, size);

    if (!is_ui && !state.camera_frustum.intersects(aabb)) return;
    if (is_ui && !state.ui_frustum.intersects(aabb)) return;

    state.shape_batch.draw_shape(shape, position, size, color, border_color, border_thickness, border_radius, anchor, is_ui, depth);
}

void Renderer::DrawText(const char* text, uint32_t length, float size, const glm::vec2& position, const glm::vec3& color, FontAsset key, bool is_ui, int depth, LLGL::RenderTarget* render_target) {
    const Font& font = Assets::GetFont(key);
    
    float x = position.x;
    float y = position.y;

    const float scale = size / font.font_size;

    uint32_t order = depth;
    if (depth != -1) {
        state.main_depth_index = glm::max(state.main_depth_index, order + 1);
    } else {
        order = state.main_depth_index++;
    }

    if (is_ui) {
        order = depth;
        if (depth != -1) {
            state.ui_depth_index = glm::max(state.ui_depth_index, order + 1);
        } else {
            order = state.ui_depth_index++;
        }
    }

    for (uint32_t i = 0; i < length; ++i) {
        const char c = text[i];
        if (c == '\n') {
            y += size;
            x = position.x;
            continue;
        }

        const Glyph& ch = font.glyphs.find(c)->second;

        if (c == ' ') {
            x += (ch.advance >> 6) * scale;
            continue;
        }

        const float xpos = x + ch.bearing.x * scale;
        const float ypos = y - ch.bearing.y * scale;
        const glm::vec2 pos = glm::vec2(xpos, ypos);
        const glm::vec2 size = glm::vec2(ch.size) * scale;

        state.glyph_batch.draw_glyph(pos, size, color, font.texture, ch.texture_coords, ch.tex_size, is_ui, order, render_target);

        x += (ch.advance >> 6) * scale;
    }
}

void Renderer::Clear(LLGL::RenderTarget* render_target, long clear_flags, const LLGL::ClearValue& clear_value) {
    auto* commands = CommandBuffer();
    commands->BeginRenderPass(*render_target);
        commands->Clear(clear_flags, clear_value);
    commands->EndRenderPass();
}

RenderTarget Renderer::CreateRenderTarget(LLGL::Extent2D resolution) {
    LLGL::TextureDescriptor texture_desc;
    texture_desc.type = LLGL::TextureType::Texture2D;
    texture_desc.extent.width = resolution.width;
    texture_desc.extent.height = resolution.height;
    texture_desc.extent.depth = 1;
    texture_desc.format = LLGL::Format::RGBA8UNorm;
    texture_desc.miscFlags = 0;

    LLGL::Texture* texture = state.context->CreateTexture(texture_desc);

    LLGL::RenderTargetDescriptor desc;
    desc.colorAttachments[0].texture = texture;
    desc.resolution = resolution;

    RenderTarget render_target;
    render_target.texture_descriptor = texture_desc;
    render_target.texture = texture;
    render_target.internal = state.context->CreateRenderTarget(desc);

    return render_target;
}

void Renderer::ResizeRenderTarget(RenderTarget* render_target, LLGL::Extent2D new_size) {
    auto& context = state.context;

    if (render_target->internal != nullptr) context->Release(*render_target->internal);
    if (render_target->texture != nullptr) context->Release(*render_target->texture);

    LLGL::TextureDescriptor texture_desc = render_target->texture_descriptor;
    texture_desc.extent.width = new_size.width;
    texture_desc.extent.height = new_size.height;
    texture_desc.extent.depth = 1;
    LLGL::Texture* texture = context->CreateTexture(texture_desc);

    LLGL::RenderTargetDescriptor desc;
    desc.colorAttachments[0].texture = texture;
    desc.resolution = new_size;
    LLGL::RenderTarget* new_render_target = context->CreateRenderTarget(desc);

    render_target->texture = texture;
    render_target->internal = new_render_target;
}

void Renderer::Release(RenderTarget* render_target) {
    auto& context = state.context;

    if (render_target->internal != nullptr) {
        context->Release(*render_target->internal);
        render_target->internal = nullptr;
    }

    if (render_target->texture != nullptr) {
        context->Release(*render_target->texture);
        render_target->texture = nullptr;
    }
}

void Renderer::Terminate() {
    Assets::DestroyShaders();

    state.sprite_batch.terminate();
    state.glyph_batch.terminate();
    state.shape_batch.terminate();

    if (state.constant_buffer) state.context->Release(*state.constant_buffer);

    if (state.command_buffer) state.context->Release(*state.command_buffer);
    if (state.swap_chain) state.context->Release(*state.swap_chain);

    Assets::DestroyTextures();
    Assets::DestroySamplers();

    LLGL::RenderSystem::Unload(std::move(state.context));
}

//
// ------------------ Batch ---------------------
//

void RenderBatchShape::init() {
    const auto& context = state.context;

    m_buffer = new ShapeInstance[MAX_QUADS];

    const BaseVertex vertices[] = {
        BaseVertex(0.0f, 0.0f),
        BaseVertex(0.0f, 1.0f),
        BaseVertex(1.0f, 0.0f),
        BaseVertex(1.0f, 1.0f),
    };

    m_vertex_buffer = CreateVertexBufferInit(sizeof(vertices), vertices, Assets::GetVertexFormat(VertexFormatAsset::ShapeVertex), "ShapeBatch VertexBuffer");
    m_instance_buffer = CreateVertexBuffer(MAX_QUADS * sizeof(ShapeInstance), Assets::GetVertexFormat(VertexFormatAsset::ShapeInstance), "ShapeBatch InstanceBuffer");

    LLGL::Buffer* buffers[] = { m_vertex_buffer, m_instance_buffer };
    m_buffer_array = context->CreateBufferArray(2, buffers);

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = {
        LLGL::BindingDescriptor(
            "GlobalUniformBuffer",
            LLGL::ResourceType::Buffer,
            LLGL::BindFlags::ConstantBuffer,
            LLGL::StageFlags::VertexStage,
            LLGL::BindingSlot(2)
        )
    };

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    const ShaderPipeline& shape_shader = Assets::GetShader(ShaderAsset::ShapeShader);

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineLayoutDesc.debugName = "ShapeBatch Pipeline";
    pipelineDesc.vertexShader = shape_shader.vs;
    pipelineDesc.fragmentShader = shape_shader.ps;
    pipelineDesc.geometryShader = shape_shader.gs;
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.indexFormat = LLGL::Format::Undefined;
    pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineDesc.renderPass = state.swap_chain->GetRenderPass();
    pipelineDesc.rasterizer.frontCCW = true;
    pipelineDesc.depth = LLGL::DepthDescriptor {
        .testEnabled = true,
        .writeEnabled = true,
        .compareOp = LLGL::CompareOp::GreaterEqual
    };
    pipelineDesc.blend = LLGL::BlendDescriptor {
        .targets = {
            LLGL::BlendTargetDescriptor {
                .blendEnabled = true,
                .srcColor = LLGL::BlendOp::SrcAlpha,
                .dstColor = LLGL::BlendOp::InvSrcAlpha,
                .srcAlpha = LLGL::BlendOp::Zero,
                .dstAlpha = LLGL::BlendOp::One,
                .alphaArithmetic = LLGL::BlendArithmetic::Max
            }
        }
    };

    m_pipeline = context->CreatePipelineState(pipelineDesc);

    if (const LLGL::Report* report = m_pipeline->GetReport()) {
        if (report->HasErrors()) LOG_ERROR("%s", report->GetText());
    }
}

void RenderBatchShape::draw_shape(Shape::Type shape, glm::vec2 position, glm::vec2 size, const glm::vec4& color, const glm::vec4& border_color, float border_thickness, float border_radius, Anchor anchor, bool is_ui, int depth) {
    if (m_count >= MAX_QUADS) {
        render();
        begin();
    }

    uint32_t& depth_index = state.main_depth_index;

    uint32_t order = depth;
    if (depth != -1) {
        depth_index = glm::max(depth_index, order + 1);
    } else {
        order = depth_index++;
    }

    if (is_ui) {
        order = depth;
        if (depth != -1) {
            state.ui_depth_index = glm::max(state.ui_depth_index, order + 1);
        } else {
            order = state.ui_depth_index++;
        }
    }

    int flags = 0;
    if (is_ui) flags |= ShapeFlags::UI;

    m_buffer_ptr->position = glm::vec3(position, static_cast<float>(order));
    m_buffer_ptr->size = size;
    m_buffer_ptr->offset = anchor.to_vec2();
    m_buffer_ptr->color = color;
    m_buffer_ptr->border_color = border_color;
    m_buffer_ptr->border_thickness = border_thickness;
    m_buffer_ptr->border_radius = border_radius;
    m_buffer_ptr->flags = flags;
    m_buffer_ptr->shape = shape;
    m_buffer_ptr++;

    m_count++;
}

void RenderBatchShape::render() {
    if (m_count == 0) return;

    auto* const commands = state.command_buffer;

    const ptrdiff_t size = (uint8_t*) m_buffer_ptr - (uint8_t*) m_buffer;
    if (size <= (1 << 16)) {
        commands->UpdateBuffer(*m_instance_buffer, 0, m_buffer, size);
    } else {
        Renderer::Context()->WriteBuffer(*m_instance_buffer, 0, m_buffer, size);
    }
    
    commands->SetVertexBufferArray(*m_buffer_array);

    commands->SetPipelineState(*m_pipeline);
    commands->SetResource(0, *state.constant_buffer);

    commands->DrawInstanced(4, 0, m_count, 0);
}

void RenderBatchShape::begin() {
    m_buffer_ptr = m_buffer;
    m_count = 0;
}

void RenderBatchShape::terminate() {
    if (m_vertex_buffer) state.context->Release(*m_vertex_buffer);
    if (m_pipeline) state.context->Release(*m_pipeline);

    delete[] m_buffer;
}



void RenderBatchSprite::init() {
    const auto& context = state.context;
    const RenderBackend backend = state.backend;

    m_buffer = new SpriteInstance[MAX_QUADS];

    const BaseVertex vertices[] = {
        BaseVertex(0.0f, 0.0f),
        BaseVertex(0.0f, 1.0f),
        BaseVertex(1.0f, 0.0f),
        BaseVertex(1.0f, 1.0f),
    };

    m_vertex_buffer = CreateVertexBufferInit(sizeof(vertices), vertices, Assets::GetVertexFormat(VertexFormatAsset::SpriteVertex), "SpriteBatch VertexBuffer");
    m_instance_buffer = CreateVertexBuffer(MAX_QUADS * sizeof(SpriteInstance), Assets::GetVertexFormat(VertexFormatAsset::SpriteInstance), "SpriteBatch InstanceBuffer");

    LLGL::Buffer* buffers[] = { m_vertex_buffer, m_instance_buffer };
    m_buffer_array = context->CreateBufferArray(2, buffers);

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = {
        LLGL::BindingDescriptor(
            "GlobalUniformBuffer",
            LLGL::ResourceType::Buffer,
            LLGL::BindFlags::ConstantBuffer,
            LLGL::StageFlags::VertexStage,
            LLGL::BindingSlot(2)
        ),
        LLGL::BindingDescriptor("u_texture", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, LLGL::BindingSlot(3)),
        LLGL::BindingDescriptor("u_sampler", LLGL::ResourceType::Sampler, 0, LLGL::StageFlags::FragmentStage, LLGL::BindingSlot(backend.IsOpenGL() ? 3 : 4))
    };

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    const ShaderPipeline& sprite_shader = Assets::GetShader(ShaderAsset::SpriteShader);

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineLayoutDesc.debugName = "SpriteBatch Pipeline";
    pipelineDesc.vertexShader = sprite_shader.vs;
    pipelineDesc.fragmentShader = sprite_shader.ps;
    pipelineDesc.geometryShader = sprite_shader.gs;
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.indexFormat = LLGL::Format::Undefined;
    pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineDesc.renderPass = state.swap_chain->GetRenderPass();
    pipelineDesc.rasterizer.frontCCW = true;
    pipelineDesc.depth = LLGL::DepthDescriptor {
        .testEnabled = true,
        .writeEnabled = true,
        .compareOp = LLGL::CompareOp::GreaterEqual
    };
    pipelineDesc.blend = LLGL::BlendDescriptor {
        .targets = {
            LLGL::BlendTargetDescriptor {
                .blendEnabled = true,
                .srcColor = LLGL::BlendOp::SrcAlpha,
                .dstColor = LLGL::BlendOp::InvSrcAlpha,
                .srcAlpha = LLGL::BlendOp::Zero,
                .dstAlpha = LLGL::BlendOp::One,
                .alphaArithmetic = LLGL::BlendArithmetic::Max
            }
        }
    };

    m_pipeline = context->CreatePipelineState(pipelineDesc);

    if (const LLGL::Report* report = m_pipeline->GetReport()) {
        if (report->HasErrors()) LOG_ERROR("%s", report->GetText());
    }
}

void RenderBatchSprite::draw_sprite(const BaseSprite& sprite, const glm::vec4& uv_offset_scale, const tl::optional<Texture>& sprite_texture, RenderLayer layer, bool is_ui, int depth, LLGL::RenderTarget* render_target) {
    if (m_sprites.size() >= MAX_QUADS) {
        render();
        begin();
    }

    std::vector<SpriteData>& sprites = m_sprites;
    uint32_t& depth_index = state.main_depth_index;

    uint32_t order = depth;
    if (depth != -1) {
        depth_index = glm::max(depth_index, order + 1);
    } else {
        order = depth_index++;
    }

    if (is_ui) {
        order = depth;
        if (depth != -1) {
            state.ui_depth_index = glm::max(state.ui_depth_index, order + 1);
        } else {
            order = state.ui_depth_index++;
        }
    }

    sprites.push_back(SpriteData {
        .position = sprite.position(),
        .rotation = sprite.rotation(),
        .size = sprite.size(),
        .offset = sprite.anchor().to_vec2(),
        .uv_offset_scale = uv_offset_scale,
        .color = sprite.color(),
        .outline_color = sprite.outline_color(),
        .outline_thickness = sprite.outline_thickness(),
        .texture = sprite_texture.is_some() ? sprite_texture.get() : Texture(),
        .render_to = render_target,
        .order = order,
        .is_ui = is_ui,
    });
}

void RenderBatchSprite::render() {
    std::vector<SpriteData>& sprites = m_sprites;

    if (sprites.empty()) return;

    std::sort(
        sprites.begin(),
        sprites.end(),
        [](const SpriteData& a, const SpriteData& b) {
            if (!a.is_ui && b.is_ui) return true;
            if (a.is_ui && !b.is_ui) return false;

            return a.texture.id < b.texture.id;
        }
    );
    
    Texture prev_texture = sprites[0].texture;
    LLGL::RenderTarget* prev_render_to = sprites[0].render_to;
    uint32_t sprite_count = 0;
    uint32_t total_sprite_count = 0;
    int vertex_offset = 0;

    for (const SpriteData& sprite_data : sprites) {
        const int prev_texture_id = prev_texture.id;
        const int curr_texture_id = sprite_data.texture.id;

        if (prev_texture_id >= 0 && prev_texture_id != curr_texture_id) {
            m_sprite_flush_queue.push_back(FlushData {
                .texture = prev_texture,
                .render_to = prev_render_to,
                .offset = vertex_offset,
                .count = sprite_count
            });
            sprite_count = 0;
            vertex_offset = total_sprite_count;
        }

        const LLGL::RenderTarget* curr_render_to = sprite_data.render_to;

        if (prev_render_to != curr_render_to) {
            m_sprite_flush_queue.push_back(FlushData {
                .texture = prev_texture,
                .render_to = prev_render_to,
                .offset = vertex_offset,
                .count = sprite_count
            });
            sprite_count = 0;
            vertex_offset = total_sprite_count;
        }

        const uint32_t order = sprite_data.is_ui ? sprite_data.order + state.main_depth_index : sprite_data.order;

        int flags = 0;
        flags |= sprite_data.is_ui << SpriteFlags::UI;
        flags |= (curr_texture_id >= 0) << SpriteFlags::HasTexture;

        m_buffer_ptr->position = glm::vec3(sprite_data.position, static_cast<float>(order));
        m_buffer_ptr->rotation = sprite_data.rotation;
        m_buffer_ptr->size = sprite_data.size;
        m_buffer_ptr->offset = sprite_data.offset;
        m_buffer_ptr->uv_offset_scale = sprite_data.uv_offset_scale;
        m_buffer_ptr->color = sprite_data.color;
        m_buffer_ptr->outline_color = sprite_data.outline_color;
        m_buffer_ptr->outline_thickness = sprite_data.outline_thickness;
        m_buffer_ptr->flags = flags;
        m_buffer_ptr++;

        sprite_count += 1;
        total_sprite_count += 1;

        prev_texture = sprite_data.texture;
    }

    m_sprite_flush_queue.push_back(FlushData {
        .texture = prev_texture,
        .render_to = prev_render_to,
        .offset = vertex_offset,
        .count = sprite_count
    });

    flush();
}

void RenderBatchSprite::flush() {
    auto* const commands = state.command_buffer;

    const ptrdiff_t size = (uint8_t*) m_buffer_ptr - (uint8_t*) m_buffer;
    if (size <= (1 << 16)) {
        commands->UpdateBuffer(*m_instance_buffer, 0, m_buffer, size);
    } else {
        Renderer::Context()->WriteBuffer(*m_instance_buffer, 0, m_buffer, size);
    }
    
    commands->SetVertexBufferArray(*m_buffer_array);

    commands->SetPipelineState(*m_pipeline);
    commands->SetResource(0, *state.constant_buffer);

    bool begin_default_render_pass = false;

    for (const FlushData& flush_data : m_sprite_flush_queue) {
        if (flush_data.render_to != nullptr) {
            commands->EndRenderPass();
            commands->BeginRenderPass(*flush_data.render_to);
            begin_default_render_pass = true;
        }

        const Texture& t = flush_data.texture.id >= 0 ? flush_data.texture : Assets::GetTexture(TextureAsset::Stub);

        commands->SetResource(1, *t.texture);
        commands->SetResource(2, Assets::GetSampler(t.sampler));
        commands->DrawInstanced(4, 0, flush_data.count, flush_data.offset);

        if (flush_data.render_to != nullptr) commands->EndRenderPass();
    }

    if (begin_default_render_pass) {
        commands->BeginRenderPass(*Renderer::SwapChain(), state.load_render_pass);
    }
}

void RenderBatchSprite::begin() {
    m_buffer_ptr = m_buffer;

    m_sprite_flush_queue.clear();
    m_sprites.clear();
}

void RenderBatchSprite::terminate() {
    if (m_vertex_buffer) state.context->Release(*m_vertex_buffer);
    if (m_pipeline) state.context->Release(*m_pipeline);

    delete[] m_buffer;
}



void RenderBatchGlyph::init() {
    const RenderBackend backend = state.backend;
    const auto& context = state.context;

    m_buffer = new GlyphInstance[MAX_QUADS];

    const BaseVertex vertices[] = {
        BaseVertex(0.0f, 0.0f),
        BaseVertex(0.0f, 1.0f),
        BaseVertex(1.0f, 0.0f),
        BaseVertex(1.0f, 1.0f),
    };

    m_vertex_buffer = CreateVertexBufferInit(sizeof(vertices), vertices, Assets::GetVertexFormat(VertexFormatAsset::FontVertex), "GlyphBatch VertexBuffer");
    m_instance_buffer = CreateVertexBuffer(MAX_QUADS * sizeof(GlyphInstance), Assets::GetVertexFormat(VertexFormatAsset::FontInstance), "GlyphBatch InstanceBuffer");

    LLGL::Buffer* buffers[] = { m_vertex_buffer, m_instance_buffer };
    m_buffer_array = context->CreateBufferArray(2, buffers);

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = {
        LLGL::BindingDescriptor(
            "GlobalUniformBuffer",
            LLGL::ResourceType::Buffer,
            LLGL::BindFlags::ConstantBuffer,
            LLGL::StageFlags::VertexStage,
            LLGL::BindingSlot(2)
        ),
        LLGL::BindingDescriptor("u_texture", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, LLGL::BindingSlot(3)),
        LLGL::BindingDescriptor("u_sampler", LLGL::ResourceType::Sampler, 0, LLGL::StageFlags::FragmentStage, LLGL::BindingSlot(backend.IsOpenGL() ? 3 : 4)),
    };

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    const ShaderPipeline& font_shader = Assets::GetShader(ShaderAsset::FontShader);

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineLayoutDesc.debugName = "GlyphBatch Pipeline";
    pipelineDesc.vertexShader = font_shader.vs;
    pipelineDesc.fragmentShader = font_shader.ps;
    pipelineDesc.geometryShader = font_shader.gs;
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.indexFormat = LLGL::Format::R32UInt;
    pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineDesc.renderPass = state.swap_chain->GetRenderPass();
    pipelineDesc.rasterizer.frontCCW = true;
    pipelineDesc.depth = LLGL::DepthDescriptor {
        .testEnabled = true,
        .writeEnabled = true,
        .compareOp = LLGL::CompareOp::GreaterEqual
    };
    pipelineDesc.blend = LLGL::BlendDescriptor {
        .targets = {
            LLGL::BlendTargetDescriptor {
                .blendEnabled = true,
                .srcColor = LLGL::BlendOp::SrcAlpha,
                .dstColor = LLGL::BlendOp::InvSrcAlpha,
                .srcAlpha = LLGL::BlendOp::Zero,
                .dstAlpha = LLGL::BlendOp::One
            }
        }
    };

    m_pipeline = context->CreatePipelineState(pipelineDesc);

    if (const LLGL::Report* report = m_pipeline->GetReport()) {
        if (report->HasErrors()) LOG_ERROR("%s", report->GetText());
    }
}

void RenderBatchGlyph::draw_glyph(const glm::vec2& pos, const glm::vec2& size, const glm::vec3& color, const Texture& font_texture, const glm::vec2& tex_uv, const glm::vec2& tex_size, bool ui, uint32_t depth, LLGL::RenderTarget* render_to = nullptr) {
    if (m_glyphs.size() >= MAX_QUADS) {
        render();
        begin();
    }

    m_glyphs.push_back(GlyphData {
        .texture = font_texture,
        .render_to = render_to,
        .color = color,
        .pos = pos,
        .size = size,
        .tex_size = tex_size,
        .tex_uv = tex_uv,
        .order = depth,
        .is_ui = ui
    });
}

void RenderBatchGlyph::render() {
    if (is_empty()) return;

    std::vector<GlyphData> sorted_glyphs = m_glyphs;
    std::sort(
        sorted_glyphs.begin(),
        sorted_glyphs.end(),
        [](const GlyphData& a, const GlyphData& b) {
            return a.texture.id < b.texture.id;
        }
    );
    
    Texture prev_texture = sorted_glyphs[0].texture;
    LLGL::RenderTarget* prev_render_to = sorted_glyphs[0].render_to;
    uint32_t sprite_count = 0;
    uint32_t total_sprite_count = 0;
    int vertex_offset = 0;

    for (const GlyphData& glyph_data : sorted_glyphs) {
        if (prev_texture.id >= 0 && prev_texture.id != glyph_data.texture.id) {
            m_glyphs_flush_queue.push_back(FlushData {
                .texture = prev_texture,
                .render_to = prev_render_to,
                .offset = vertex_offset,
                .count = sprite_count
            });
            sprite_count = 0;
            vertex_offset = total_sprite_count;
        }

        if (prev_render_to != glyph_data.render_to) {
            m_glyphs_flush_queue.push_back(FlushData {
                .texture = prev_texture,
                .render_to = prev_render_to,
                .offset = vertex_offset,
                .count = sprite_count
            });
            sprite_count = 0;
            vertex_offset = total_sprite_count;
        }

        const float order = glyph_data.is_ui ? glyph_data.order + state.main_depth_index : glyph_data.order;

        m_buffer_ptr->color = glyph_data.color;
        m_buffer_ptr->pos = glm::vec3(glyph_data.pos, static_cast<float>(order));
        m_buffer_ptr->size = glyph_data.size;
        m_buffer_ptr->tex_size = glyph_data.tex_size;
        m_buffer_ptr->uv = glyph_data.tex_uv;
        m_buffer_ptr->is_ui = glyph_data.is_ui;
        m_buffer_ptr++;

        sprite_count += 1;
        total_sprite_count += 1;

        prev_texture = glyph_data.texture;
    }

    m_glyphs_flush_queue.push_back(FlushData {
        .texture = prev_texture,
        .render_to = prev_render_to,
        .offset = vertex_offset,
        .count = sprite_count
    });

    flush();
}

void RenderBatchGlyph::flush() {
    auto* const commands = state.command_buffer;

    const ptrdiff_t size = (uint8_t*) m_buffer_ptr - (uint8_t*) m_buffer;
    if (size <= (1 << 16)) {
        commands->UpdateBuffer(*m_instance_buffer, 0, m_buffer, size);
    } else {
        Renderer::Context()->WriteBuffer(*m_instance_buffer, 0, m_buffer, size);
    }
    
    commands->SetVertexBufferArray(*m_buffer_array);

    commands->SetPipelineState(*m_pipeline);
    commands->SetResource(0, *state.constant_buffer);

    bool begin_default_render_pass = false;

    for (const FlushData& flush_data : m_glyphs_flush_queue) {
        const Texture& t = flush_data.texture;

        if (flush_data.render_to != nullptr) {
            commands->EndRenderPass();
            commands->BeginRenderPass(*flush_data.render_to);
            begin_default_render_pass = true;
        }

        commands->SetResource(1, *t.texture);
        commands->SetResource(2, Assets::GetSampler(t.sampler));
        commands->DrawInstanced(4, 0, flush_data.count, flush_data.offset);

        if (flush_data.render_to != nullptr) commands->EndRenderPass();
    }

    if (begin_default_render_pass) commands->BeginRenderPass(*state.swap_chain, state.load_render_pass);
}

void RenderBatchGlyph::begin() {
    m_buffer_ptr = m_buffer;
    m_glyphs_flush_queue.clear();
    m_glyphs.clear();
}

void RenderBatchGlyph::terminate() {
    if (m_vertex_buffer) state.context->Release(*m_vertex_buffer);
    if (m_pipeline) state.context->Release(*m_pipeline);

    delete[] m_buffer;
}