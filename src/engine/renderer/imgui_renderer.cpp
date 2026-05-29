#include "imgui_renderer.hpp"

#include <LLGL/PipelineStateFlags.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/TextureFlags.h>
#include <SGE/renderer/types.hpp>
#include <SGE/types/binding_layout.hpp>
#include <SGE/types/attributes.hpp>
#include <SGE/types/window_settings.hpp>
#include <SGE/defines.hpp>

#include <GLFW/glfw3.h>

#if SGE_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <backends/imgui_impl_glfw.h>

#include "imgui.h"

#include "shaders.hpp"

struct BackendData {
    LLGL::VertexFormat vertexFormat;

    std::vector<LLGL::SwapChainDescriptor> swapChainDescsForViewports;

    std::shared_ptr<sge::RenderContext> context;
    sge::Unique<LLGL::Buffer> constantBuffer;
    sge::Unique<LLGL::Buffer> indexBuffer;
    sge::Unique<LLGL::Buffer> vertexBuffer;
    LLGL::CommandBuffer* commandBuffer = nullptr;

    sge::Ref<LLGL::PipelineLayout> pipelineLayout;

    sge::Handle<LLGL::PipelineState> pipelineHandle;

    uint32_t vertexBufferSize = 0;
    uint32_t indexBufferSize = 0;
};

struct FrameRenderBuffers
{
    sge::Unique<LLGL::Buffer> VertexBuffer;
    sge::Unique<LLGL::Buffer> IndexBuffer;
};

// Each viewport will hold 1 ImGui_ImplVulkanH_WindowRenderBuffers
// [Please zero-clear before use!]
struct WindowRenderBuffers
{
    uint32_t Index = 0;
    uint32_t Count = 0;
    std::vector<FrameRenderBuffers> FrameRenderBuffers;
};

struct ViewportData
{
    sge::Ref<LLGL::SwapChain> SwapChain;
    std::shared_ptr<sge::GlfwWindow> Window;
    bool WindowOwned = false;
};

struct TextureData
{
    sge::Texture texture;
};

static BackendData* GetBackendData()
{
    return ImGui::GetCurrentContext() ? (BackendData*)ImGui::GetIO().BackendRendererUserData : nullptr;
}


void DrawCallback_ResetRenderState(const ImDrawList*, const ImDrawCmd*) {}

void DrawCallback_SetSamplerLinear(const ImDrawList* parent_list, const ImDrawCmd* cmd) {
    BackendData* bd = GetBackendData();
    bd->commandBuffer->SetResource(2, *bd->context->GetLinearSampler());
}

void DrawCallback_SetSamplerNearest(const ImDrawList* parent_list, const ImDrawCmd* cmd) {
    BackendData* bd = GetBackendData();
    bd->commandBuffer->SetResource(2, *bd->context->GetNearestSampler());
}

static std::shared_ptr<sge::GlfwWindow> CreateGlfwWindow(sge::WindowSettings window_settings) {
    glfwWindowHint(GLFW_FOCUSED, 1);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, window_settings.hidden ? GLFW_FALSE : GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 1);
    glfwWindowHint(GLFW_RESIZABLE, window_settings.resizable ? GLFW_TRUE : GLFW_FALSE);

    GLFWmonitor* primary_monitor = window_settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

    GLFWwindow *window = glfwCreateWindow(window_settings.width, window_settings.height, window_settings.title, primary_monitor, nullptr);
    if (window == nullptr) {
        return nullptr;
    }

    glfwSetInputMode(window, GLFW_CURSOR, static_cast<int>(window_settings.cursor_mode));

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glm::ivec2 position;
    glfwGetWindowPos(window, &position.x, &position.y);

    const uint8_t vsync_interval = window_settings.vsync;

    std::shared_ptr<sge::GlfwWindow> instance = std::make_shared<sge::GlfwWindow>(window, LLGL::Extent2D(width, height), position, window_settings.cursor_mode, window_settings.samples, vsync_interval, window_settings.fullscreen);
    return instance;
}

static void UpdateTexture(ImTextureData* tex) {
    BackendData* bd = GetBackendData();

    if (tex->Status == ImTextureStatus_WantCreate)
    {
        // Create texture based on tex->Width, tex->Height.
        // - Most backends only support tex->Format == ImTextureFormat_RGBA32.
        // - Backends for particularly memory constrained platforms may support tex->Format == ImTextureFormat_Alpha8.

        // Upload all texture pixels
        // - Read from our CPU-side copy of the texture and copy to your graphics API.
        // - Use tex->Width, tex->Height, tex->GetPixels(), tex->GetPixelsAt(), tex->GetPitch() as needed.

        TextureData* backend_tex = IM_NEW(TextureData)();

        LLGL::TextureDescriptor textureDesc;
        textureDesc.type = LLGL::TextureType::Texture2D;
        textureDesc.miscFlags = LLGL::MiscFlags::FixedSamples;
        textureDesc.extent.width = tex->Width;
        textureDesc.extent.height = tex->Height;
        
        LLGL::ImageView imageView;
        imageView.data = tex->GetPixels();
        imageView.dataSize = tex->GetSizeInBytes();

        if (tex->Format == ImTextureFormat_RGBA32) {
            textureDesc.format = LLGL::Format::RGBA8UNorm;
            imageView.format = LLGL::ImageFormat::RGBA;
        } else if (tex->Format == ImTextureFormat_Alpha8) {
            textureDesc.format = LLGL::Format::A8UNorm;
            imageView.format = LLGL::ImageFormat::Alpha;
        }

        uint8_t* data = static_cast<uint8_t*>(tex->GetPixels());

        backend_tex->texture = bd->context->CreateTexture(textureDesc.type, imageView.format, LLGL::DataType::UInt8, textureDesc.extent.width, textureDesc.extent.height, textureDesc.extent.depth, bd->context->GetNearestSampler(), data);

        // Store your data, and acknowledge creation.
        tex->SetTexID((ImTextureID)(intptr_t)&backend_tex->texture); // Specify backend-specific ImTextureID identifier which will be stored in ImDrawCmd.
        tex->SetStatus(ImTextureStatus_OK);
        tex->BackendUserData = backend_tex; // Store more backend data if needed (most backend allocate a small texture to store data in there)
    }
    if (tex->Status == ImTextureStatus_WantUpdates)
    {
        // Upload a rectangle of pixels to the existing texture
        // - We only ever write to textures regions which have never been used before!
        // - Use tex->TexID or tex->BackendUserData to retrieve your stored data.
        // - Use tex->UpdateRect.x/y, tex->UpdateRect.w/h to obtain the block position and size.
        //   - Use tex->Updates[] to obtain individual sub-regions within tex->UpdateRect. Not recommended.
        // - Read from our CPU-side copy of the texture and copy to your graphics API.
        // - Use tex->Width, tex->Height, tex->GetPixels(), tex->GetPixelsAt(), tex->GetPitch() as needed.

        TextureData* backend_tex = static_cast<TextureData*>(tex->BackendUserData);

        LLGL::TextureRegion region(LLGL::Offset3D(tex->UpdateRect.x, tex->UpdateRect.y, 0), LLGL::Extent3D(tex->UpdateRect.w, tex->UpdateRect.h, 1));

        LLGL::ImageView srcImageView;
        srcImageView.data = tex->GetPixelsAt(tex->UpdateRect.x, tex->UpdateRect.y);
        srcImageView.rowStride = tex->GetPitch();
        srcImageView.dataSize = tex->UpdateRect.w * tex->UpdateRect.h * tex->BytesPerPixel;

        bd->context->GetLLGLContext()->WriteTexture(*backend_tex->texture.internal(), region, srcImageView);

        // Acknowledge update
        tex->SetStatus(ImTextureStatus_OK);
    }
    if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > 0)
    {
        // If you use staged rendering and have in-flight renders, changed tex->UnusedFrames > 0 check to higher count as needed e.g. > 2

        TextureData* backend_tex = static_cast<TextureData*>(tex->BackendUserData);
        IM_DELETE(backend_tex);

        // Acknowledge destruction
        tex->SetTexID(ImTextureID_Invalid);
        tex->SetStatus(ImTextureStatus_Destroyed);
    }
}

static bool CreatePipelineObjects() {
    BackendData* bd = GetBackendData();

    LLGL::PipelineLayoutDescriptor layoutDesc;
    layoutDesc.bindings = sge::BindingLayout({
        sge::BindingLayoutItem::ConstantBuffer(2, "UniformBuffer", LLGL::StageFlags::VertexStage),
        sge::BindingLayoutItem::Texture(3, "Texture", LLGL::StageFlags::FragmentStage),
        sge::BindingLayoutItem::Sampler(4, "Sampler", LLGL::StageFlags::FragmentStage),
    });
    bd->pipelineLayout = bd->context->CreatePipelineLayout(layoutDesc);

    bd->vertexFormat = sge::Attributes(bd->context->Backend(), {
        sge::Attribute::Vertex(LLGL::Format::RG32Float, "a_position", "Position"),
        sge::Attribute::Vertex(LLGL::Format::RG32Float, "a_uv", "UV"),
        sge::Attribute::Vertex(LLGL::Format::RGBA8UNorm, "a_color", "Color"),
    });

    ShaderSourceCode shader = GetImguiShaderSourceCode(bd->context->Backend());
    sge::Ref<LLGL::Shader> vertexShader = bd->context->CreateShader(sge::ShaderType::Vertex, "VS", shader.vs_source, shader.vs_size, bd->vertexFormat.attributes);
    sge::Ref<LLGL::Shader> pixelShader = bd->context->CreateShader(sge::ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);

    sge::GraphicsPipelineConfig config;
    {
        config.layout = bd->pipelineLayout;
        config.vertexShader = std::move(vertexShader);
        config.pixelShader = std::move(pixelShader);
        config.indexFormat = sizeof(ImDrawIdx) == 2 ? LLGL::Format::R16UInt : LLGL::Format::R32UInt;
        config.cullMode = LLGL::CullMode::Disabled;
        config.scissorTestEnabled = true;
        config.depth.testEnabled = false;
        config.depth.writeEnabled = false;
        config.blend.targets[0] = LLGL::BlendTargetDescriptor {
            .blendEnabled = true
        };
    }

    bd->pipelineHandle = bd->context->CreatePipelineState(config);
    bd->constantBuffer = bd->context->CreateConstantBuffer(sizeof(glm::mat4));

    return true;
}

static void DestroyPipelineObjects() {
    BackendData* bd = GetBackendData();
    bd->context->DeletePipeline(bd->pipelineHandle);
}

static void ImGui_ImplGlfw_SwapBuffers(ImGuiViewport* viewport, void*)
{
    glfwSwapBuffers((GLFWwindow*)viewport->PlatformHandle);
}

bool ImGuiRenderer::Init(std::shared_ptr<sge::RenderContext> context) {
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    BackendData* bd = IM_NEW(BackendData)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = "imgui_impl_sge";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;   // We can honor ImGuiPlatformIO::Textures[] requests during render.
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Renderer_TextureMaxWidth = context->GetRenderingCaps().limits.max2DTextureSize;
    platform_io.Renderer_TextureMaxHeight = context->GetRenderingCaps().limits.max2DTextureSize;
    platform_io.DrawCallback_ResetRenderState = DrawCallback_ResetRenderState;
    platform_io.DrawCallback_SetSamplerLinear = DrawCallback_SetSamplerLinear;
    platform_io.DrawCallback_SetSamplerNearest = DrawCallback_SetSamplerNearest;
    platform_io.Platform_SwapBuffers = ImGui_ImplGlfw_SwapBuffers;

    bd->commandBuffer = context->GetCommandBuffer();
    bd->context = std::move(context);

    if (!CreatePipelineObjects())
        return false;

    return true;
}

void ImGuiRenderer::Shutdown() {
    BackendData* bd = GetBackendData();
    IM_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");

    ImGuiIO& io = ImGui::GetIO();
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

    DestroyPipelineObjects();

    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures | ImGuiBackendFlags_RendererHasViewports);
    platform_io.ClearRendererHandlers();
    IM_DELETE(bd);
}

void ImGuiRenderer::NewFrame() {
    BackendData* bd = GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplDX11_Init()?");

    if (!bd->pipelineHandle.IsValid())
        if (!CreatePipelineObjects())
            IM_ASSERT(0 && "ImGui_ImplDX11_CreateDeviceObjects() failed!");
}

static void SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height) {
    BackendData* bd = GetBackendData();

    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    float mvp[4][4] =
    {
        { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
        { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
        { 0.0f,         0.0f,           0.5f,       0.0f },
        { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
    };

    bd->commandBuffer->UpdateBuffer(*bd->constantBuffer, 0, mvp, sizeof(mvp));

    bd->commandBuffer->SetViewport(LLGL::Extent2D(fb_width, fb_height));
    bd->commandBuffer->SetPipelineState(bd->context->GetOrCreatePipeline(bd->pipelineHandle));
    bd->commandBuffer->SetVertexBuffer(*bd->vertexBuffer);
    bd->commandBuffer->SetIndexBuffer(*bd->indexBuffer);
    bd->commandBuffer->SetResource(0, *bd->constantBuffer);
}

void ImGuiRenderer::RenderDrawData(ImDrawData* draw_data) {
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;

    if (draw_data->Textures != nullptr)
        for (ImTextureData* tex : *draw_data->Textures)
            if (tex->Status != ImTextureStatus_OK)
                UpdateTexture(tex);

    BackendData* bd = GetBackendData();

    if (!bd->vertexBuffer || bd->vertexBufferSize < draw_data->TotalVtxCount)
    {
        bd->vertexBufferSize = draw_data->TotalVtxCount + 5000;

        LLGL::BufferDescriptor desc;
        desc.miscFlags = LLGL::MiscFlags::DynamicUsage | LLGL::MiscFlags::NoInitialData;
        desc.size = bd->vertexBufferSize * sizeof(ImDrawVert);
        desc.bindFlags = LLGL::BindFlags::VertexBuffer;
        desc.cpuAccessFlags = LLGL::CPUAccessFlags::Write;
        desc.vertexAttribs = bd->vertexFormat.attributes;

        bd->vertexBuffer = bd->context->CreateBuffer(desc);
    }
    if (!bd->indexBuffer || bd->indexBufferSize < draw_data->TotalIdxCount)
    {
        bd->indexBufferSize = draw_data->TotalIdxCount + 10000;

        LLGL::BufferDescriptor desc;
        desc.miscFlags = LLGL::MiscFlags::DynamicUsage | LLGL::MiscFlags::NoInitialData;
        desc.size = bd->indexBufferSize * sizeof(ImDrawIdx);
        desc.bindFlags = LLGL::BindFlags::IndexBuffer;
        desc.cpuAccessFlags = LLGL::CPUAccessFlags::Write;
        desc.format = sizeof(ImDrawIdx) == 2 ? LLGL::Format::R16UInt : LLGL::Format::R32UInt;
        
        bd->indexBuffer = bd->context->CreateBuffer(desc);
    }

    void* vtxData = bd->context->GetLLGLContext()->MapBuffer(*bd->vertexBuffer, LLGL::CPUAccess::WriteDiscard);
    if (!vtxData)
        return;

    void* idxData = bd->context->GetLLGLContext()->MapBuffer(*bd->indexBuffer, LLGL::CPUAccess::WriteDiscard);
    if (!idxData) {
        bd->context->GetLLGLContext()->UnmapBuffer(*bd->vertexBuffer);
        return;
    }

    ImDrawVert* vtx_dst = static_cast<ImDrawVert*>(vtxData);
    ImDrawIdx* idx_dst = static_cast<ImDrawIdx*>(idxData);
    for (const ImDrawList* draw_list : draw_data->CmdLists)
    {
        memcpy(vtx_dst, draw_list->VtxBuffer.Data, draw_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idx_dst, draw_list->IdxBuffer.Data, draw_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtx_dst += draw_list->VtxBuffer.Size;
        idx_dst += draw_list->IdxBuffer.Size;
    }
    bd->context->GetLLGLContext()->UnmapBuffer(*bd->vertexBuffer);
    bd->context->GetLLGLContext()->UnmapBuffer(*bd->indexBuffer);

    SetupRenderState(draw_data, fb_width, fb_height);

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_idx_offset = 0;
    int global_vtx_offset = 0;
    ImVec2 clip_off = draw_data->DisplayPos;
    ImVec2 clip_scale = draw_data->FramebufferScale;
    for (const ImDrawList* draw_list : draw_data->CmdLists)
    {
        const ImDrawVert* vtx_buffer = draw_list->VtxBuffer.Data;  // vertex buffer generated by Dear ImGui
        const ImDrawIdx* idx_buffer = draw_list->IdxBuffer.Data;   // index buffer generated by Dear ImGui
        for (int cmd_i = 0; cmd_i < draw_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &draw_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                if (pcmd->UserCallback == DrawCallback_ResetRenderState)
                    SetupRenderState(draw_data, fb_width, fb_height);
                else
                    pcmd->UserCallback(draw_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                // - Clipping coordinates are provided in imgui coordinates space:
                //   - For a given viewport, draw_data->DisplayPos == viewport->Pos and draw_data->DisplaySize == viewport->Size
                //   - In a single viewport application, draw_data->DisplayPos == (0,0) and draw_data->DisplaySize == io.DisplaySize, but always use GetMainViewport()->Pos/Size instead of hardcoding those values.
                //   - In the interest of supporting multi-viewport applications (see 'docking' branch on github),
                //     always subtract draw_data->DisplayPos from clipping bounds to convert them to your viewport space.
                // - Note that pcmd->ClipRect contains Min+Max bounds. Some graphics API may use Min+Max, other may use Min+Size (size being Max-Min)
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                // We are using scissoring to clip some objects. All low-level graphics API should support it.
                // - If your engine doesn't support scissoring yet, you may ignore this at first. You will get some small glitches
                //   (some elements visible outside their bounds) but you can fix t
                const float clip_width = clip_max.x - clip_min.x;
                const float clip_height = clip_max.y - clip_min.y;

                bd->commandBuffer->SetScissor(LLGL::Scissor(clip_min.x, clip_min.y, clip_width, clip_height));

                // The texture for the draw call is specified by pcmd->GetTexID().
                // The vast majority of draw calls will use the Dear ImGui texture atlas, which value you have set yourself during initialization.
                sge::Texture* texture = reinterpret_cast<sge::Texture*>(pcmd->GetTexID());

                bd->commandBuffer->SetResource(1, *texture->internal());
                bd->commandBuffer->SetResource(2, *texture->sampler());

                // Render 'pcmd->ElemCount/3' indexed triangles.
                // By default the indices ImDrawIdx are 16-bit, you can change them to 32-bit in imconfig.h if your engine doesn't support 16-bit indices.
                bd->commandBuffer->DrawIndexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset);
            }
        }

        global_idx_offset += draw_list->IdxBuffer.Size;
        global_vtx_offset += draw_list->VtxBuffer.Size;
    }
    
    bd->commandBuffer->SetScissor(LLGL::Scissor(0, 0, (uint32_t)fb_width, (uint32_t)fb_height));
}