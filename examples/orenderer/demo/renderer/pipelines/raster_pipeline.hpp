#ifndef RASTER_PIPELINE_HPP
#define RASTER_PIPELINE_HPP

#include <orenderer/render_pipeline.hpp>
#include <orendergraph/resource_handle.hpp>
#include <ogfx/buffer.hpp>

#include "../features/deferred_lighting_feature.hpp"
#include "../features/to_swapchain_feature.hpp"

#include "../../scene/geometry_generation.hpp"

struct SceneGeometry;
struct DirectionalLight;

namespace ogfx
{

class Swapchain;

}

struct RasterPipelineDesc
{
    DeferredLightingDesc deferred_feature_desc;
    ToSwapchainDesc to_swapchain_feature_desc;
};

class RasterPipeline : public orenderer::RenderPipeline
{
  public:
    using orenderer::RenderPipeline::RenderPipeline;

    void setup(RasterPipelineDesc& desc);
};

#endif // RASTER_PIPELINE_HPP