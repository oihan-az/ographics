#ifndef OMEGA_EDITOR_UI_PIPELINE_HPP
#define OMEGA_EDITOR_UI_PIPELINE_HPP

#include <orenderer/render_pipeline.hpp>

#include <omega/core/entity/entity.hpp>

namespace omega
{

class ComponentSystem;
class EventSystem;
class TransformSystem;
class CameraSystem;
class GraphicsSystem;
class PlatformSystem;

} // namespace omega

namespace omega_editor
{

class ImGuiLayer;

struct UIPipelineDesc
{
    omega::Entity editor_camera;

    ImGuiLayer& imgui_layer;

    omega::ComponentSystem& components;
    omega::EventSystem& events;
    omega::TransformSystem& transforms;
    omega::CameraSystem& cameras;
    omega::PlatformSystem& platform;
    omega::GraphicsSystem& graphics;
};

class UIPipeline : public orenderer::RenderPipeline
{
  public:
    using orenderer::RenderPipeline::RenderPipeline;

    void setup(UIPipelineDesc& desc);
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_UI_PIPELINE_HPP