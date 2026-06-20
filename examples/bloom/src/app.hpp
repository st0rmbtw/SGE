#ifndef APP_HPP_
#define APP_HPP_

#pragma once

#include <SGE/engine.hpp>
#include <SGE/renderer/renderer2d.hpp>
#include <SGE/types/backend.hpp>

#include <LLGL/PipelineState.h>

#include "../../common.hpp"

struct UniformBuffer {
    glm::mat4x4 view_matrix;
    glm::mat4x4 projection_matrix;
    glm::vec3 object_color;
} SGE_ALIGN(16);

class App : public sge::IEngine {
public:
    explicit App(const ExampleConfig& config) : m_config(config) {}

protected:
    bool OnInit() override;
    void OnUpdate() override;
    void OnRender(const std::shared_ptr<sge::GlfwWindow> &window) override;
    
    void OnWindowResized(const std::shared_ptr<sge::GlfwWindow> &window, int width, int height) override {
        InitFramebuffer(LLGL::Extent2D(width, height));
        OnRender(window);
    }

private:
    void InitPipeline();
    void InitFramebuffer(LLGL::Extent2D resolution);

private:
    UniformBuffer m_uniforms;

    glm::vec3 m_camera_pos = glm::vec3(0.0f);
    glm::vec3 m_camera_forward = glm::vec3(1.0, 0.0, 0.0);

    glm::vec3 m_clear_color = glm::vec3(0.4f, 0.4f, 0.4f);

    sge::BloomSettings m_bloom_settings;

    float m_yaw = 0.0f;
    float m_pitch = 0.0f;

    std::unique_ptr<sge::Framebuffer> m_postprocess_framebuffer = nullptr;

    sge::Ref<LLGL::Buffer> m_vertex_buffer;
    sge::Ref<LLGL::Buffer> m_uniform_buffer;

    sge::Handle<LLGL::PipelineState> m_pipeline_handle;
    sge::Handle<LLGL::PipelineState> m_postprocess_pipeline_handle;

    std::unique_ptr<sge::Renderer2D> m_renderer;
    ExampleConfig m_config;
};

#endif