#ifndef APP_HPP
#define APP_HPP

#include <SGE/engine.hpp>
#include <SGE/renderer/renderer2d.hpp>
#include <SGE/time/stopwatch.hpp>
#include <SGE/types/color.hpp>
#include <SGE/types/shape.hpp>

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
    void DrawContent(LLGL::Extent2D viewport);

private:
    sge::Camera m_camera;
    std::unique_ptr<sge::Renderer2D> m_renderer;
    std::unique_ptr<sge::Batch> m_batch;
    ExampleConfig m_config;

    sge::LinearRgba m_custom_color = sge::LinearRgba::white();

    uint32_t m_instance_count = 0;
    uint32_t m_batch_limit = 0;
    uint32_t m_primary_window_id = 0;

    BatchType m_batch_type = BatchType::Line;
    sge::Shape::Type m_shape_type = sge::Shape::Rect;
    Coloring m_coloring = Coloring::Random;
    glm::uvec2 m_size_from = glm::uvec2(0);
    glm::uvec2 m_size_to = glm::uvec2(500);
    float m_radius_from = 1.0f;
    float m_radius_to = 250.0f;
    float m_outer_radius_from = 1.0f;
    float m_outer_radius_to = 250.0f;
    float m_inner_radius_from = 1.0f;
    float m_inner_radius_to = 250.0f;
};

#endif