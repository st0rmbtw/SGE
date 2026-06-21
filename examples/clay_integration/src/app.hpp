#ifndef APP_HPP_
#define APP_HPP_

#pragma once

#include <SGE/engine.hpp>
#include <SGE/renderer/renderer2d.hpp>
#include <SGE/types/backend.hpp>

#include "../../common.hpp"

class App : public sge::IEngine {
public:
    explicit App(const ExampleConfig& config) : m_config(config) {}
    ~App();

protected:
    bool OnInit() override;
    void OnUpdate() override;
    void OnRender(const std::shared_ptr<sge::GlfwWindow> &window) override;
    void OnPostRender(const std::shared_ptr<sge::GlfwWindow> &window) override;
    
    void OnWindowResized(const std::shared_ptr<sge::GlfwWindow>&, int width, int height) override {
        m_camera.set_viewport(glm::uvec2(width, height));
    }

private:
    std::unique_ptr<sge::Renderer2D> m_renderer;
    std::unique_ptr<sge::Batch> m_batch;
    sge::Camera m_camera;
    ExampleConfig m_config;
};

#endif