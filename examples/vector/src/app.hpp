#ifndef APP_HPP
#define APP_HPP

#include <SGE/engine.hpp>
#include <SGE/renderer/renderer2d.hpp>

#include "../../common.hpp"

class App : public sge::IEngine {
public:
    explicit App(const ExampleConfig& config) : m_config(config) {}
    ~App();

protected:
    bool OnInit() override;
    void OnUpdate() override;
    void OnRender(const std::shared_ptr<sge::GlfwWindow> &window) override;

    void OnWindowResized(const std::shared_ptr<sge::GlfwWindow>&, int width, int height) override {
        m_camera.set_viewport(glm::uvec2(width, height));
    }

private:
    sge::Camera m_camera;
    sge::Path m_path;
    std::unique_ptr<sge::Renderer2D> m_renderer;
    ExampleConfig m_config;
    bool m_paused = false;
};

#endif
