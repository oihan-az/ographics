#ifndef RASTER_PIPELINE_HPP
#define RASTER_PIPELINE_HPP

#include <orenderer/render_pipeline.hpp>

#include <omega/core/entity/entity.hpp>
#include <omega/texture/texture_handle.hpp>
#include <omega/texture/texture_view_handle.hpp>

namespace omega
{

class ComponentSystem;
class EventSystem;
class TransformSystem;
class CameraSystem;
class GraphicsSystem;

struct RasterPipelineDesc
{
    Entity camera;

    ComponentSystem& components;
    EventSystem& events;
    TransformSystem& transforms;
    CameraSystem& cameras;
    GraphicsSystem& graphics;
};

class RasterPipeline : public orenderer::RenderPipeline
{
  public:
    using orenderer::RenderPipeline::RenderPipeline;

    void setup(RasterPipelineDesc& desc);
};

}

#endif // RASTER_PIPELINE_HPP