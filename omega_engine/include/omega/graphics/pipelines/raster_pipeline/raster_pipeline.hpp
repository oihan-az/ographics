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
class GpuMeshSystem;
class GpuTextureSystem;

struct RasterPipelineDesc
{
    Entity camera;
    uint32_t width = 0;
    uint32_t height = 0;

    uint32_t frames_in_flight = 0;
    uint32_t* current_frame = nullptr;

    std::vector<orendergraph::ResourceHandle<ogfx::Image>> swapchain_images;
    uint32_t* swapchain_image_index = nullptr;

    TextureHandle missing_texture;
    TextureViewHandle missing_texture_view;

    ComponentSystem& components;
    EventSystem& events;
    TransformSystem& transforms;
    CameraSystem& cameras;
    GpuMeshSystem& gpu_meshes;
    GpuTextureSystem& gpu_textures;
};

class RasterPipeline : public orenderer::RenderPipeline
{
  public:
    using orenderer::RenderPipeline::RenderPipeline;

    void setup(RasterPipelineDesc& desc);
};

}

#endif // RASTER_PIPELINE_HPP