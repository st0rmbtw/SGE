#ifndef APP_HPP_
#define APP_HPP_

#pragma once

#include <SGE/engine.hpp>
#include <SGE/types/backend.hpp>
#include <SGE/renderer/renderer.hpp>

#include "../../common.hpp"

class App : public sge::IEngine {
public:
    App(const ExampleConfig& config) : m_config(config) {}
    ~App();

    bool Init() override;
protected:
    void OnUpdate() override;
    void OnRender(const std::shared_ptr<sge::GlfwWindow> &window) override;
    void OnPostRender(const std::shared_ptr<sge::GlfwWindow> &window) override;
    
    void OnWindowResized(const std::shared_ptr<sge::GlfwWindow> &window, int width, int height) override {
        m_camera.set_viewport(glm::uvec2(width, height));
        m_camera.update();
        OnRender(window);
    }
    
    void OnWindowDestroy(sge::GlfwWindow &window) override {
        if (window.GetID() == m_primary_window_id) {
            Stop();
        }
    }
private:
    std::unique_ptr<sge::Renderer> m_renderer;
    std::unique_ptr<sge::Batch> m_batch;
    sge::Camera m_camera;
    ExampleConfig m_config;
    uint32_t m_primary_window_id = 0;
};

#endif