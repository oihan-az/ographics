#ifndef GEOMETRY_RENDER_FEATURE_HPP
#define GEOMETRY_RENDER_FEATURE_HPP

#include <orenderer/render_feature.hpp>

#include <omega/core/entity/entity.hpp>
#include <orendergraph/resource_handle.hpp>

#include <cstdint>

namespace ogfx
{
class Image;
class ImageView;
} // namespace ogfx

namespace orendergraph
{
class RenderGraph;
} // namespace orendergraph

namespace omega
{

class ComponentSystem;
class TransformSystem;
class CameraSystem;
class GpuMeshSystem;

struct GeometryRenderFeatureDesc
{
    Entity camera;
    uint32_t width = 0;
    uint32_t height = 0;

    ComponentSystem& components;
    TransformSystem& transforms;
    CameraSystem& cameras;
    GpuMeshSystem& gpu_meshes;
};

class GeometryRenderFeature : public orenderer::IRenderFeature
{
  public:
    GeometryRenderFeature(const GeometryRenderFeatureDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;

    const orendergraph::ResourceHandle<ogfx::Image> out_image() const { return m_out_image; }

  private:
    GeometryRenderFeatureDesc m_desc;

    void create_resources(orendergraph::RenderGraph& rg);
    void add_node(orendergraph::RenderGraph& rg);

    orendergraph::ResourceHandle<ogfx::Image> m_out_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_out_image_view;

    orendergraph::ResourceHandle<ogfx::Image> m_depth_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_depth_image_view;
};

}

#endif // GEOMETRY_RENDER_FEATURE_HPP