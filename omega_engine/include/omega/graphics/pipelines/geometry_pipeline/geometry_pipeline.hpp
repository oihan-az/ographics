#ifndef GEOMTRY_RENDER_PIPELINE_HPP
#define GEOMTRY_RENDER_PIPELINE_HPP

#include <orenderer/render_pipeline.hpp>

#include <omega/graphics/pipelines/geometry_pipeline/features/geometry_feature.hpp>

namespace omega
{

struct GeometryRenderPipelineDesc
{
    GeometryRenderFeatureDesc geomtry_render_desc;
};

class GeometryRenderPipeline : public orenderer::RenderPipeline
{
  public:
    using orenderer::RenderPipeline::RenderPipeline;

    void setup(GeometryRenderPipelineDesc& desc);
};

}

#endif // GEOMTRY_RENDER_PIPELINE_HPP