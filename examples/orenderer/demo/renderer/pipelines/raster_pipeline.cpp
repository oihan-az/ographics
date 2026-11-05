#include "raster_pipeline.hpp"

void RasterPipeline::setup(RasterPipelineDesc& desc)
{
    begin_setup();

    // Deferred feature
    ogfx::Buffer* vertex_buffer = m_shared_registry.get_resource(desc.deferred_feature_desc.vertex_buffer);
    desc.deferred_feature_desc.vertex_buffer = m_render_graph.import_resource(vertex_buffer);

    ogfx::Buffer* index_buffer = m_shared_registry.get_resource(desc.deferred_feature_desc.index_buffer);
    desc.deferred_feature_desc.index_buffer = m_render_graph.import_resource(index_buffer);

    DeferredLighting& deferred_feature = add_feature<DeferredLighting>(desc.deferred_feature_desc);

    // To swapchain feature
    desc.to_swapchain_feature_desc.image = deferred_feature.lighting_image();
    for (size_t i = 0; i < desc.to_swapchain_feature_desc.swapchain_images.size(); i++)
    {
        ogfx::Image* swapchain_image = m_shared_registry.get_resource(desc.to_swapchain_feature_desc.swapchain_images[i]);
        desc.to_swapchain_feature_desc.swapchain_images[i] = m_render_graph.import_resource(swapchain_image);
    }

    add_feature<ToSwapchain>(desc.to_swapchain_feature_desc);
}