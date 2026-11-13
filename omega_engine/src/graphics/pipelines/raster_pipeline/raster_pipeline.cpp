#include <omega/graphics/pipelines/raster_pipeline/raster_pipeline.hpp>
#include <omega/graphics/pipelines/raster_pipeline/features/geometry_feature.hpp>
#include <omega/graphics/pipelines/raster_pipeline/features/to_swapchain_feature.hpp>

namespace omega
{

void RasterPipeline::setup(RasterPipelineDesc& desc)
{
    GeometryRenderFeatureDesc geomtry_render_desc
    {
        desc.camera,
        desc.width, 
        desc.height,
        desc.frames_in_flight,
        desc.current_frame,
        desc.components,
        desc.events,
        desc.transforms,
        desc.cameras,
        desc.gpu_meshes,
        desc.gpu_textures
    };
    
    GeometryRenderFeature& geometry_render_feature = add_feature<GeometryRenderFeature>(geomtry_render_desc);

    ToSwapchainDesc to_swapchain_desc;
    to_swapchain_desc.swapchain_width = desc.width;
    to_swapchain_desc.swapchain_height = desc.height;
    to_swapchain_desc.image_index = desc.swapchain_image_index;

    // To swapchain feature
    to_swapchain_desc.image = geometry_render_feature.out_image();
    to_swapchain_desc.swapchain_images.resize(desc.swapchain_images.size());
    for (size_t i = 0; i < desc.swapchain_images.size(); i++)
    {
        ogfx::Image* swapchain_image = m_shared_registry.get_resource(desc.swapchain_images[i]);
        to_swapchain_desc.swapchain_images[i] = m_render_graph.import_resource(swapchain_image);
    }

    add_feature<ToSwapchain>(to_swapchain_desc);
}

}