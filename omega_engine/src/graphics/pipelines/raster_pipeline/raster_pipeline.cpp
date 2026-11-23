#include <omega/graphics/pipelines/raster_pipeline/raster_pipeline.hpp>
#include <omega/graphics/pipelines/raster_pipeline/features/geometry_feature.hpp>
#include <omega/graphics/pipelines/raster_pipeline/features/to_swapchain_feature.hpp>
#include <omega/graphics/graphics_system.hpp>

namespace omega
{

void RasterPipeline::setup(RasterPipelineDesc& desc)
{
    //GeometryRenderFeatureDesc geometry_render_desc
    //{
    //    desc.camera,
    //    desc.camera_type,
    //    desc.components,
    //    desc.events,
    //    desc.transforms,
    //    desc.cameras,
    //    desc.graphics,
    //};
    //
    //GeometryRenderFeature& geometry_render_feature = add_feature<GeometryRenderFeature>(geometry_render_desc);

    //ToSwapchainDesc to_swapchain_desc
    //{
    //    geometry_render_feature.out_image(),
    //    desc.graphics
    //};
    //
    //add_feature<ToSwapchain>(to_swapchain_desc);
}

} // namespace omega