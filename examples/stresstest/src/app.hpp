#ifndef APP_HPP
#define APP_HPP

#include <SGE/engine.hpp>
#include <SGE/renderer/renderer.hpp>
#include <SGE/time/stopwatch.hpp>
#include <SGE/types/color.hpp>

#include "../../common.hpp"

enum class BatchType : uint8_t {
    Line = 0,
    Shape,
    NinePatch,
    Sprite
};

enum class Coloring : uint8_t {
    Random = 0,
    Custom
};

class App : public sge::IEngine {
public:
    App(const ExampleConfig& config) : m_config(config) {}
    ~App();

protected:
    bool OnInit() override;
    void OnUpdate() override;
    void OnRender(const std::shared_ptr<sge::GlfwWindow> &window) override;

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
    sge::Camera m_camera;
    std::unique_ptr<sge::Renderer> m_renderer;
    std::unique_ptr<sge::Batch> m_batch;
    ExampleConfig m_config;

    sge::LinearRgba m_custom_color = sge::LinearRgba::white();

    size_t m_instance_count = 0;
    uint32_t m_primary_window_id = 0;

    BatchType m_batch_type = BatchType::Line;
    Coloring m_coloring = Coloring::Random;
    bool m_paused = false;
};

#endif