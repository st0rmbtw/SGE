#include "imgui_renderer.hpp"

#include <LLGL/PipelineStateFlags.h>
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/Surface.h>
#include <LLGL/TextureFlags.h>

#include <SGE/assert.hpp>
#include <SGE/defines.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/utils.hpp>
#include <SGE/types/attributes.hpp>
#include <SGE/types/binding_layout.hpp>
#include <SGE/types/window_settings.hpp>

#include <GLFW/glfw3.h>
#include <unordered_map>

#if SGE_PLATFORM_WINDOWS
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif SGE_PLATFORM_MACOS
    #define GLFW_EXPOSE_NATIVE_COCOA
#else
    #if SGE_PLATFORM_LINUX
        #define GLFW_EXPOSE_NATIVE_WAYLAND
        #define GLFW_EXPOSE_NATIVE_X11
    #endif
#endif

#include <GLFW/glfw3native.h>

#include <backends/imgui_impl_glfw.h>

#include "imgui.h"

#include "shaders.hpp"

// Undefine the Status macro defined in Xlib
#undef Status

namespace {
struct BackendData {
    LLGL::VertexFormat VertexFormat;

    std::shared_ptr<sge::RenderContext> Context;
    sge::Unique<LLGL::Buffer> ConstantBuffer;
    sge::Unique<LLGL::Buffer> IndexBuffer;
    sge::Unique<LLGL::Buffer> VertexBuffer;
    LLGL::CommandBuffer* CommandBuffer = nullptr;

    sge::Ref<LLGL::PipelineLayout> PipelineLayout;

    sge::Handle<LLGL::PipelineState> PipelineHandle;

    uint32_t VertexBufferSize = 5000;
    uint32_t IndexBufferSize = 10000;

    uint32_t GlobalIdxOffset = 0;
    uint32_t GlobalVtxOffset = 0;

    uint8_t* VertexDataBuffer = nullptr;
    uint8_t* IndexDataBuffer = nullptr;
};

struct TextureData
{
    sge::Texture texture;
    sge::TextureWithSampler tws;
};

class GlfwSurface : public LLGL::Surface {
public:
    GlfwSurface(GLFWwindow* window, LLGL::Extent2D size) : m_size(size), m_wnd(window) {
    }

    [[nodiscard]]
    LLGL::Display* FindResidentDisplay() const override {
        return LLGL::Display::GetPrimary();
    }

    [[nodiscard]]
    LLGL::Extent2D GetContentSize() const override {
        return m_size;
    }

    bool GetNativeHandle(void *nativeHandle, std::size_t) override {
        auto* handle = reinterpret_cast<LLGL::NativeHandle*>(nativeHandle);
        #if defined(SGE_PLATFORM_WINDOWS)
            handle->window = glfwGetWin32Window(m_wnd);
        #elif defined(SGE_PLATFORM_MACOS)
            handle->responder = glfwGetCocoaWindow(m_wnd);
        #elif defined(SGE_PLATFORM_LINUX)
            int platform = glfwGetPlatform();

            if (platform == GLFW_PLATFORM_WAYLAND) {
                handle->wayland.window = glfwGetWaylandWindow(m_wnd);
                handle->wayland.display = glfwGetWaylandDisplay();
                handle->type = LLGL::NativeType::Wayland;
            } else {
                handle->x11.window = glfwGetX11Window(m_wnd);
                handle->x11.display = glfwGetX11Display();
                handle->type = LLGL::NativeType::X11;
            }

        #endif
        return true;
    }

    bool AdaptForVideoMode(LLGL::Extent2D *resolution, bool *fullscreen) override {
        bool result = true;
        if (resolution != nullptr) {
            glfwSetWindowSize(m_wnd, resolution->width, resolution->height);
            uint32_t width = 0;
            uint32_t height = 0;
            {
                int w, h;
                glfwGetWindowSize(m_wnd, &w, &h);
                width = w;
                height = h;
            }
            if (resolution->width != width || resolution->height != height) {
                resolution->width = width;
                resolution->height = height;
                result = false;
            }
            m_size = *resolution;
        }
        if (fullscreen != nullptr) {
            GLFWmonitor* monitor = *fullscreen ? glfwGetPrimaryMonitor() : nullptr;
            glfwSetWindowMonitor(m_wnd, monitor, 0, 0, m_size.width, m_size.height, GLFW_DONT_CARE);
            if (glfwGetWindowMonitor(m_wnd) != monitor) {
                result = false;
            }
        }
        return result;
    }
private:
    LLGL::Extent2D m_size;
    GLFWwindow* m_wnd = nullptr;
};

std::unordered_map<GLFWwindow*, sge::Unique<LLGL::SwapChain>> g_SwapChainMap = {};
std::unordered_map<GLFWwindow*, std::shared_ptr<GlfwSurface>> g_WindowMap = {};

BackendData* GetBackendData()
{
    return (ImGui::GetCurrentContext() != nullptr) ? static_cast<BackendData*>(ImGui::GetIO().BackendRendererUserData) : nullptr;
}

void DrawCallback_ResetRenderState(const ImDrawList*, const ImDrawCmd*) {}

void DrawCallback_SetSamplerLinear(const ImDrawList*, const ImDrawCmd*) {
    BackendData* bd = GetBackendData();
    bd->CommandBuffer->SetResource(2, *bd->Context->GetLinearSampler());
}

void DrawCallback_SetSamplerNearest(const ImDrawList*, const ImDrawCmd*) {
    BackendData* bd = GetBackendData();
    bd->CommandBuffer->SetResource(2, *bd->Context->GetNearestSampler());
}

void UpdateTexture(ImTextureData* tex) {
    BackendData* bd = GetBackendData();

    if (tex->Status == ImTextureStatus_WantCreate)
    {
        // Create texture based on tex->Width, tex->Height.
        // - Most backends only support tex->Format == ImTextureFormat_RGBA32.
        // - Backends for particularly memory constrained platforms may support tex->Format == ImTextureFormat_Alpha8.

        // Upload all texture pixels
        // - Read from our CPU-side copy of the texture and copy to your graphics API.
        // - Use tex->Width, tex->Height, tex->GetPixels(), tex->GetPixelsAt(), tex->GetPitch() as needed.

        auto* backend_tex = IM_NEW(TextureData)();

        sge::TextureConfig textureConfig;
        textureConfig.textureType = LLGL::TextureType::Texture2D;
        textureConfig.extent.width = tex->Width;
        textureConfig.extent.height = tex->Height;
        textureConfig.sampler = bd->Context->GetNearestSampler();
        
        LLGL::ImageView imageView;
        imageView.data = tex->GetPixels();
        imageView.dataSize = tex->GetSizeInBytes();

        if (tex->Format == ImTextureFormat_RGBA32) {
            textureConfig.format = LLGL::Format::RGBA8UNorm;
            imageView.format = LLGL::ImageFormat::RGBA;
        } else if (tex->Format == ImTextureFormat_Alpha8) {
            textureConfig.format = LLGL::Format::A8UNorm;
            imageView.format = LLGL::ImageFormat::Alpha;
        }

        backend_tex->texture = bd->Context->CreateTexture(textureConfig, &imageView);
        backend_tex->tws.texture = backend_tex->texture;
        backend_tex->tws.sampler = backend_tex->texture.sampler()->internal();

        // Store your data, and acknowledge creation.
        tex->SetTexID((ImTextureID)(intptr_t)&backend_tex->tws); // Specify backend-specific ImTextureID identifier which will be stored in ImDrawCmd.
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

        auto* backend_tex = static_cast<TextureData*>(tex->BackendUserData);

        LLGL::TextureRegion region(LLGL::Offset3D(tex->UpdateRect.x, tex->UpdateRect.y, 0), LLGL::Extent3D(tex->UpdateRect.w, tex->UpdateRect.h, 1));

        LLGL::ImageView srcImageView;
        srcImageView.data = tex->GetPixelsAt(tex->UpdateRect.x, tex->UpdateRect.y);
        srcImageView.rowStride = tex->GetPitch();
        srcImageView.dataSize = tex->UpdateRect.w * tex->UpdateRect.h * tex->BytesPerPixel;

        bd->Context->GetLLGLContext()->WriteTexture(*backend_tex->texture.internal(), region, srcImageView);

        // Acknowledge update
        tex->SetStatus(ImTextureStatus_OK);
    }
    if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > 0)
    {
        // If you use staged rendering and have in-flight renders, changed tex->UnusedFrames > 0 check to higher count as needed e.g. > 2

        auto* backend_tex = static_cast<TextureData*>(tex->BackendUserData);
        IM_DELETE(backend_tex);

        // Acknowledge destruction
        tex->SetTexID(ImTextureID_Invalid);
        tex->SetStatus(ImTextureStatus_Destroyed);
    }
}

bool CreatePipelineObjects() {
    BackendData* bd = GetBackendData();

    LLGL::PipelineLayoutDescriptor layoutDesc;
    layoutDesc.bindings = sge::BindingLayout({
        sge::BindingLayoutItem::ConstantBuffer(2, "UniformBuffer_std140", LLGL::StageFlags::VertexStage),
        sge::BindingLayoutItem::Texture(3, "Texture", LLGL::StageFlags::FragmentStage),
        sge::BindingLayoutItem::Sampler(4, "Sampler", LLGL::StageFlags::FragmentStage),
    });
    layoutDesc.combinedTextureSamplers = {
        LLGL::CombinedTextureSamplerDescriptor("Texture", "Texture", "Sampler", 3)
    };
    bd->PipelineLayout = bd->Context->CreatePipelineLayout(layoutDesc);

    bd->VertexFormat = sge::Attributes(bd->Context->Backend(), {
        sge::Attribute::Vertex(LLGL::Format::RG32Float, "inp_position", "Position"),
        sge::Attribute::Vertex(LLGL::Format::RG32Float, "inp_uv", "UV"),
        sge::Attribute::Vertex(LLGL::Format::RGBA8UNorm, "inp_color", "Color"),
    });

    ShaderSourceCode shader = GetImguiShaderSourceCode(bd->Context->Backend());

    sge::ShaderConfig shaderConfig;
    shaderConfig.vertex.inputAttribs = bd->VertexFormat.attributes;

    sge::Ref<LLGL::Shader> vertexShader = bd->Context->CreateShader(sge::ShaderType::Vertex, "VS", shader.vs_source, shader.vs_size, shaderConfig);
    sge::Ref<LLGL::Shader> pixelShader = bd->Context->CreateShader(sge::ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);

    sge::GraphicsPipelineConfig config;
    {
        config.layout = bd->PipelineLayout;
        config.vertexShader = std::move(vertexShader);
        config.pixelShader = std::move(pixelShader);
        config.indexFormat = sizeof(ImDrawIdx) == 2 ? LLGL::Format::R16UInt : LLGL::Format::R32UInt;
        config.cullMode = LLGL::CullMode::Disabled;
        config.scissorTestEnabled = true;
        config.depth.testEnabled = false;
        config.depth.writeEnabled = false;
        config.depth.compareOp = LLGL::CompareOp::AlwaysPass;
        config.blend.targets[0] = LLGL::BlendTargetDescriptor {
            .blendEnabled = true,
            .srcColor = LLGL::BlendOp::SrcAlpha,
            .dstColor = LLGL::BlendOp::InvSrcAlpha,
            .srcAlpha = LLGL::BlendOp::SrcAlpha,
            .dstAlpha = LLGL::BlendOp::InvSrcAlpha,
        };
    }

    bd->PipelineHandle = bd->Context->CreatePipelineState(config);
    bd->ConstantBuffer = bd->Context->CreateConstantBuffer(sizeof(glm::mat4));

    return true;
}

void DestroyPipelineObjects() {
    BackendData* bd = GetBackendData();
    bd->Context->DeletePipeline(bd->PipelineHandle);
}

void Renderer_CreateWindow(ImGuiViewport* viewport) {
    BackendData* bd = GetBackendData();
    
    auto* glfwHandle = static_cast<GLFWwindow*>(viewport->PlatformHandle);

    std::shared_ptr<GlfwSurface> surface = std::make_shared<GlfwSurface>(glfwHandle, LLGL::Extent2D(viewport->Size.x, viewport->Size.y));
    
    LLGL::SwapChainDescriptor swapChainDesc;
    swapChainDesc.resolution.width = viewport->Size.x;
    swapChainDesc.resolution.height = viewport->Size.y;
    swapChainDesc.depthBits = 0;
    LLGL::SwapChain* swapChain = bd->Context->GetLLGLContext()->CreateSwapChain(swapChainDesc, surface);
    swapChain->SetVsyncInterval(0);
    
    g_SwapChainMap[glfwHandle] = bd->Context->CreateUnique(swapChain);
    g_WindowMap[glfwHandle] = surface;
}

void Renderer_DestroyWindow(ImGuiViewport* viewport) {
    auto* glfwHandle = static_cast<GLFWwindow*>(viewport->PlatformHandle);

    g_SwapChainMap.erase(glfwHandle);
    g_WindowMap.erase(glfwHandle);
}

void Renderer_SwapBuffers(ImGuiViewport* viewport, void*) {
    auto it = g_SwapChainMap.find((GLFWwindow*)viewport->PlatformHandle);
    if (it == g_SwapChainMap.end()) {
        return;
    }

    it->second->Present();
}

void Renderer_ResizeWindow(ImGuiViewport* viewport, ImVec2 size) {
    auto it = g_SwapChainMap.find((GLFWwindow*)viewport->PlatformHandle);
    if (it == g_SwapChainMap.end()) {
        return;
    }

    it->second->ResizeBuffers(LLGL::Extent2D(size.x, size.y));
}

void Renderer_RenderWindow(ImGuiViewport* viewport, void*) {
    auto it = g_SwapChainMap.find((GLFWwindow*)viewport->PlatformHandle);
    if (it == g_SwapChainMap.end()) {
        return;
    }

    BackendData* bd = GetBackendData();

    bd->CommandBuffer->BeginRenderPass(*it->second);
        bd->Context->PushRenderTarget(it->second);
        {
            bd->CommandBuffer->Clear(LLGL::ClearFlags::Color, LLGL::ClearValue(0.0f, 0.0f, 0.0f, 1.0f));
            ImGuiRenderer::RenderDrawData(viewport->DrawData);
        }
        bd->Context->PopRenderTarget();
    bd->CommandBuffer->EndRenderPass();
}

void InitMultiViewportSupport() {
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Renderer_CreateWindow = Renderer_CreateWindow;
    platform_io.Renderer_DestroyWindow = Renderer_DestroyWindow;
    platform_io.Renderer_SetWindowSize = Renderer_ResizeWindow;
    platform_io.Renderer_RenderWindow = Renderer_RenderWindow;
    platform_io.Renderer_SwapBuffers = Renderer_SwapBuffers;
}

void SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height) {
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

    bd->CommandBuffer->UpdateBuffer(*bd->ConstantBuffer, 0, mvp, sizeof(mvp));
    bd->CommandBuffer->SetPipelineState(bd->Context->GetOrCreatePipeline(bd->PipelineHandle));
    bd->CommandBuffer->SetViewport(LLGL::Extent2D(fb_width, fb_height));
    bd->CommandBuffer->SetVertexBuffer(*bd->VertexBuffer);
    bd->CommandBuffer->SetIndexBuffer(*bd->IndexBuffer);
    bd->CommandBuffer->SetResource(0, *bd->ConstantBuffer);
}

} // namespace

bool ImGuiRenderer::Init(std::shared_ptr<sge::RenderContext> context) {
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    SGE_ASSERT_M(io.BackendRendererUserData == nullptr, "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    BackendData* bd = IM_NEW(BackendData)();
    io.BackendRendererUserData = static_cast<void*>(bd);
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

    bd->CommandBuffer = context->GetCommandBuffer();
    bd->Context = std::move(context);

    if (!CreatePipelineObjects())
        return false;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        InitMultiViewportSupport();

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
    SGE_ASSERT_M(bd != nullptr, "Context or backend not initialized! Did you call ImGuiRenderer::Init()?");

    if (!bd->PipelineHandle.IsValid()) {
        bool result = CreatePipelineObjects();
        SGE_ASSERT_M(result, "CreatePipelineObjects failed!");
    }

    bd->GlobalVtxOffset = 0;
    bd->GlobalIdxOffset = 0;
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

    if (!bd->VertexBuffer || bd->VertexBufferSize < bd->GlobalVtxOffset + draw_data->TotalVtxCount)
    {
        bd->VertexBufferSize = bd->GlobalVtxOffset + draw_data->TotalVtxCount + 5000;

        LLGL::BufferDescriptor desc;
        desc.miscFlags = LLGL::MiscFlags::DynamicUsage | LLGL::MiscFlags::NoInitialData;
        desc.size = bd->VertexBufferSize * sizeof(ImDrawVert);
        desc.bindFlags = LLGL::BindFlags::VertexBuffer;
        desc.vertexAttribs = bd->VertexFormat.attributes;

        bd->VertexBuffer = bd->Context->CreateBuffer(desc);
        bd->VertexDataBuffer = new uint8_t[bd->VertexBufferSize * sizeof(ImDrawVert)];
    }
    if (!bd->IndexBuffer || bd->IndexBufferSize < bd->GlobalIdxOffset + draw_data->TotalIdxCount)
    {
        bd->IndexBufferSize = bd->GlobalIdxOffset + draw_data->TotalIdxCount + 10000;

        LLGL::BufferDescriptor desc;
        desc.miscFlags = LLGL::MiscFlags::DynamicUsage | LLGL::MiscFlags::NoInitialData;
        desc.size = bd->IndexBufferSize * sizeof(ImDrawIdx);
        desc.bindFlags = LLGL::BindFlags::IndexBuffer;
        desc.format = sizeof(ImDrawIdx) == 2 ? LLGL::Format::R16UInt : LLGL::Format::R32UInt;
        
        bd->IndexBuffer = bd->Context->CreateBuffer(desc);
        bd->IndexDataBuffer = new uint8_t[bd->IndexBufferSize * sizeof(ImDrawIdx)];
    }

    ImDrawVert* vtxStart = reinterpret_cast<ImDrawVert*>(bd->VertexDataBuffer) + bd->GlobalVtxOffset;
    ImDrawIdx* idxStart = reinterpret_cast<ImDrawIdx*>(bd->IndexDataBuffer) + bd->GlobalIdxOffset;

    ImDrawVert* vtx_dst = vtxStart;
    ImDrawIdx* idx_dst = idxStart;
    for (const ImDrawList* draw_list : draw_data->CmdLists)
    {
        memcpy(vtx_dst, draw_list->VtxBuffer.Data, draw_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idx_dst, draw_list->IdxBuffer.Data, draw_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtx_dst += draw_list->VtxBuffer.Size;
        idx_dst += draw_list->IdxBuffer.Size;
    }

    uintptr_t vtxLength = (vtx_dst - vtxStart) * sizeof(ImDrawVert);
    uintptr_t idxLength = (idx_dst - idxStart) * sizeof(ImDrawIdx);

    uintptr_t vtxOffset = bd->GlobalVtxOffset * sizeof(ImDrawVert);
    uintptr_t idxOffset = bd->GlobalIdxOffset * sizeof(ImDrawIdx);

    sge::UpdateBufferChunked(*bd->CommandBuffer, *bd->VertexBuffer, vtxOffset, bd->VertexDataBuffer, vtxLength);
    sge::UpdateBufferChunked(*bd->CommandBuffer, *bd->IndexBuffer, idxOffset, bd->IndexDataBuffer, idxLength);

    SetupRenderState(draw_data, fb_width, fb_height);

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    ImVec2 clip_off = draw_data->DisplayPos;
    ImVec2 clip_scale = draw_data->FramebufferScale;
    for (const ImDrawList* draw_list : draw_data->CmdLists)
    {
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

                bd->CommandBuffer->SetScissor(LLGL::Scissor(clip_min.x, clip_min.y, clip_width, clip_height));

                // The texture for the draw call is specified by pcmd->GetTexID().
                // The vast majority of draw calls will use the Dear ImGui texture atlas, which value you have set yourself during initialization.
                auto* texture = reinterpret_cast<sge::TextureWithSampler*>(pcmd->GetTexID());

                LLGL::Sampler* sampler = texture->sampler == nullptr ? bd->Context->GetLinearSampler()->internal() : texture->sampler;

                bd->CommandBuffer->SetResource(1, *texture->texture);
                bd->CommandBuffer->SetResource(2, *sampler);

                // Render 'pcmd->ElemCount/3' indexed triangles.
                // By default the indices ImDrawIdx are 16-bit, you can change them to 32-bit in imconfig.h if your engine doesn't support 16-bit indices.
                bd->CommandBuffer->DrawIndexed(pcmd->ElemCount, pcmd->IdxOffset + bd->GlobalIdxOffset, pcmd->VtxOffset + bd->GlobalVtxOffset);
            }
        }

        bd->GlobalIdxOffset += draw_list->IdxBuffer.Size;
        bd->GlobalVtxOffset += draw_list->VtxBuffer.Size;
    }
    
    bd->CommandBuffer->SetScissor(LLGL::Scissor(0, 0, (uint32_t)fb_width, (uint32_t)fb_height));
}