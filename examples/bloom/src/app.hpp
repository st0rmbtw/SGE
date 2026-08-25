#ifndef APP_HPP_
#define APP_HPP_

#pragma once

#include <SGE/engine.hpp>
#include <SGE/renderer/renderer2d.hpp>
#include <SGE/types/backend.hpp>

#include <LLGL/PipelineState.h>

#include "../../common.hpp"

struct alignas(16) UniformBuffer {
    glm::mat4x4 view_matrix;
    glm::mat4x4 projection_matrix;
    glm::vec3 object_color = glm::vec3(2.5f, 0.5f, 1.0f);
};

class App : public sge::IEngine {
public:
    explicit App(const ExampleConfig& config) : m_config(config) {}

protected:
    bool OnInit() override;
    void OnUpdate() override;
    void OnRender(const std::shared_ptr<sge::GlfwWindow>& window) override;

private:
    void InitPipeline();

private:
    UniformBuffer m_uniforms;

    sge::Transform m_transform;

    glm::vec3 m_clear_color = glm::vec3(0.4f, 0.4f, 0.4f);

    sge::BloomSettings m_bloom_settings;

    float m_yaw = 0.0f;
    float m_pitch = 0.0f;

    sge::Ref<LLGL::Buffer> m_uniform_buffer;
    sge::Ref<sge::Mesh> m_mesh;
    sge::Ref<sge::Material> m_material;

    std::unique_ptr<sge::Renderer> m_renderer;
    ExampleConfig m_config;

    bool m_render_imgui = true;
};

#endif