#ifndef OMEGA_EDITOR_UI_PIPELINE_HPP
#define OMEGA_EDITOR_UI_PIPELINE_HPP

#include <orenderer/render_pipeline.hpp>

#include <omega/core/entity/entity.hpp>
#include <omega/camera/camera_type.hpp>

namespace omega
{

class ComponentSystem;
class EventSystem;
class TransformSystem;
class CameraSystem;
class GraphicsSystem;
class PlatformSystem;
class ToSwapchain;
class PbrOpaqueRenderFeature;
class EnvironmentMapGen;
class SkyboxRenderFeature;
class DitheringFeature;
class IrradianceMapGen;
class EnvironmentPrefilterFeature;
class BrdfLutGen;
class ToneMappingFeature;
struct RenderRegion;

} // namespace omega

namespace omega_editor
{

class ImGuiLayer;

struct UIPipelineDesc
{
    omega::Entity camera;
    omega::CameraType camera_type;
    omega::Entity atmosphere;

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

    void set_viewport(const omega::RenderRegion& region);
    void set_camera(omega::Entity camera, omega::CameraType camera_type);

    omega::ToSwapchain* m_to_swapchain;
    omega::PbrOpaqueRenderFeature* m_pbr_opaque_feature;
    omega::EnvironmentMapGen* m_environment_map_gen = nullptr;
    omega::SkyboxRenderFeature* m_skybox_feature = nullptr;
    omega::DitheringFeature* m_dithering_feature = nullptr;
    omega::IrradianceMapGen* m_irradiance_map_gen = nullptr;
    omega::EnvironmentPrefilterFeature* m_environment_prefilter_feature = nullptr;
    omega::BrdfLutGen* m_brdf_lut_gen = nullptr;
    omega::ToneMappingFeature* m_tone_mapping_feature = nullptr;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_UI_PIPELINE_HPP