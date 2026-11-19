#ifndef OMEGA_EDITOR_IMGUI_LAYER_HPP
#define OMEGA_EDITOR_IMGUI_LAYER_HPP

#include <memory>

namespace ogfx
{

class CommandBuffer;

} // namespace ogfx

namespace omega
{

class PlatformSystem;
class GraphicsSystem;

} // namespace omega

namespace omega_editor
{

class UIFeature;

class ImGuiLayer
{
  public:
    ImGuiLayer();
    ~ImGuiLayer();

    ImGuiLayer(const ImGuiLayer&) = delete;
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;

    void initialize(omega::PlatformSystem& platform, omega::GraphicsSystem& graphics, UIFeature& ui_render_feature);
    void begin_frame();
    void end_frame();
    void render(ogfx::CommandBuffer& command_buffer);
    void shutdown();

  private:
    bool m_initialized = false;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_IMGUI_LAYER_HPP