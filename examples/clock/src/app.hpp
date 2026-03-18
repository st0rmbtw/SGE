#ifndef APP_HPP
#define APP_HPP

#include <SGE/engine.hpp>
#include <SGE/renderer/renderer.hpp>
#include <SGE/time/stopwatch.hpp>

#include "../../common.hpp"

struct CurrentTime {
    sge::Duration::Nanos time;
    float seconds;
    float minutes;
    float hours;
};

class App : public sge::IEngine {
public:
    App(const ExampleConfig& config);
    ~App();
protected:
    void OnUpdate() override;
    void OnRender(const std::shared_ptr<sge::GlfwWindow> &window) override;
    void OnPostRender(const std::shared_ptr<sge::GlfwWindow> &window) override;
    void OnWindowResized(const std::shared_ptr<sge::GlfwWindow> &window, int width, int height) override;

    void OnWindowDestroy(sge::GlfwWindow &window) override {
        if (window.GetID() == m_primary_window_id) {
            Stop();
        }
    }

private:
    void sync_time();

private:
    sge::Camera m_camera = sge::Camera(sge::CameraOrigin::TopLeft);
    std::unique_ptr<sge::Renderer> m_renderer;
    std::unique_ptr<sge::Batch> m_batch;
    CurrentTime m_t;
    uint32_t m_primary_window_id = 0;
    bool m_paused = false;
};

#endif