#ifndef APP_HPP
#define APP_HPP

#include <SGE/engine.hpp>
#include <SGE/renderer/renderer2d.hpp>

#include "../../common.hpp"

class App : public sge::IEngine {
public:
    explicit App(const ExampleConfig& config) : m_config(config) {}

protected:
    bool OnInit() override;
    void OnUpdate() override;
    void OnRender(const std::shared_ptr<sge::GlfwWindow> &window) override;

    void OnInputEvent(const sge::InputEvent& event) override;

    void OnWindowResized(const std::shared_ptr<sge::GlfwWindow>&, int width, int height) override {
        m_camera.set_viewport(glm::uvec2(width, height));
    }

private:
    void GenerateFontTextureWithFreetype(uint32_t n, std::string* messages, uint32_t* sizes, uint32_t atlas_width = 8000, uint32_t atlas_height = 8000);

private:
    sge::Camera m_camera;
    sge::FontVector m_font;
    sge::Font m_font_sdf;
    std::shared_ptr<sge::Renderer2D> m_renderer;
    std::unique_ptr<sge::BatchGroup> m_draw_ctx;
    std::vector<std::string> m_messages;
    std::vector<uint32_t> m_sizes;
    sge::Texture m_freetype_output;
    LLGL::Extent2D m_scene_resolution;
    ExampleConfig m_config;
    int m_freetype_flags = 0;
    bool m_simulation_view_hovered = false;
    bool m_freetype_texture_should_regenerate = false;
    bool m_paused = false;
};

#endif
