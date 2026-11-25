#ifndef APP_HPP
#define APP_HPP

#include <SGE/engine.hpp>
#include <SGE/renderer/renderer2d.hpp>
#include <SGE/types/path_builder.hpp>

#include "../../common.hpp"

class App : public sge::IEngine {
public:
    explicit App(const ExampleConfig& config) : m_config(config) {}
    ~App();

protected:
    bool OnInit() override;
    void OnUpdate() override;
    void OnRender(const std::shared_ptr<sge::GlfwWindow> &window) override;

    void OnInputEvent(const sge::InputEvent &event) override;

    void OnWindowResized(const std::shared_ptr<sge::GlfwWindow>&, int width, int height) override {
        m_camera.set_viewport(glm::uvec2(width, height));
    }

private:
    sge::Camera m_camera;
    LLGL::FrameProfile m_profile;
    sge::PathBuilder m_path_builder;
    std::shared_ptr<sge::GpuVectorShape> m_vector_shape;
    std::shared_ptr<sge::Renderer2D> m_renderer;
    std::unique_ptr<sge::BatchGroup> m_batch_group;
    ExampleConfig m_config;
    bool m_paused = false;

    struct {
        LLGL::Extent2D scene_resolution{ 0, 0 };
        bool scene_hovered{ false };
    } m_imgui_state{};
};

#endif
