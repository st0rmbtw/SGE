#ifndef APP_HPP_
#define APP_HPP_

#pragma once

#include <SGE/engine.hpp>
#include <SGE/math/quaternion.hpp>
#include <SGE/renderer/renderer2d.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/types/backend.hpp>
#include <SGE/types/transform.hpp>

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

    sge::Transform m_transform = sge::Transform().WithTranslation(glm::vec3(-2.3f, 1.5f, 2.3f));

    glm::vec3 m_clear_color = sge::LinearRgba(27, 27, 27).to_vec3();

    sge::BloomSettings m_bloom_settings;
    sge::TonemapSettings m_tonemap_settings;

    float m_yaw = -45.8f;
    float m_pitch = -28.6f;

    sge::Ref<LLGL::Buffer> m_uniform_buffer;
    sge::Ref<sge::Mesh> m_mesh;
    sge::Ref<sge::Material> m_material;

    std::unique_ptr<sge::Renderer> m_renderer;
    ExampleConfig m_config;

    bool m_render_imgui = true;
    bool m_transform_changed = true;
};

#endif