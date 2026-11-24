#include <omega_editor/rendering/ui_pipeline.hpp>
#include <omega_editor/rendering/ui_feature.hpp>

#include <omega/graphics/pipelines/raster_pipeline/features/geometry_feature.hpp>
#include <omega/graphics/pipelines/raster_pipeline/features/to_swapchain_feature.hpp>
#include <omega/graphics/pipelines/raster_pipeline/features/environment_map_gen.hpp>
#include <omega/graphics/pipelines/raster_pipeline/features/skybox_feature.hpp>

#include <omega_editor/rendering/ui_feature.hpp>

namespace omega_editor
{

void UIPipeline::setup(UIPipelineDesc& desc)
{
    omega::EnvironmentMapGenDesc environment_desc
    {
        desc.camera,
        desc.atmosphere,
        desc.components,
        desc.transforms,
        desc.graphics
    };

    m_environment_map_gen = &add_feature<omega::EnvironmentMapGen>(environment_desc);

    omega::PbrOpaqueRenderFeatureDesc pbr_opaque_desc
    {
        desc.camera,
        desc.camera_type,
        desc.components,
        desc.events,
        desc.transforms,
        desc.cameras,
        desc.graphics,
    };
    
    m_pbr_opaque_feature = &add_feature<omega::PbrOpaqueRenderFeature>(pbr_opaque_desc);

    omega::SkyboxRenderFeatureDesc skybox_desc
    {
        desc.camera,
        desc.camera_type,
    
        m_environment_map_gen->environment_map(),
        m_environment_map_gen->environment_map_view(),
    
        m_pbr_opaque_feature->out_image(),
        m_pbr_opaque_feature->out_image_view(),
    
        m_pbr_opaque_feature->depth_image(),
        m_pbr_opaque_feature->depth_image_view(),
    
        desc.cameras,
        desc.graphics
    };
    
    m_skybox_feature = &add_feature<omega::SkyboxRenderFeature>(skybox_desc);

    omega::ToSwapchainDesc to_swapchain_desc
    {
        m_pbr_opaque_feature->out_image(),
        desc.graphics
    };

    m_to_swapchain = &add_feature<omega::ToSwapchain>(to_swapchain_desc);

    UIFeatureDesc ui_feature_desc
    {
        m_to_swapchain->swapchain_images(),
        desc.imgui_layer,
        desc.platform,
        desc.graphics
    };

    add_feature<UIFeature>(ui_feature_desc);
}

void UIPipeline::set_viewport(const omega::RenderRegion& region)
{
    m_to_swapchain->set_destination_region(region);
}

void UIPipeline::set_camera(omega::Entity camera, omega::CameraType camera_type)
{
    m_environment_map_gen->set_camera(camera);
    m_pbr_opaque_feature->set_camera(camera, camera_type);
    m_skybox_feature->set_camera(camera, camera_type);
}

} // namespace omega_editor