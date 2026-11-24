#ifndef OMEGA_SKYBOX_FEATURE_HPP
#define OMEGA_SKYBOX_FEATURE_HPP

#include <orenderer/render_feature.hpp>

#include <orendergraph/resource_handle.hpp>

#include <omega/camera/camera_type.hpp>
#include <omega/core/entity/entity.hpp>

namespace ogfx
{
class Image;
class ImageView;
class Sampler;
} // namespace ogfx

namespace orendergraph
{
class RenderGraph;
template <typename TContext> class RenderGraphNode;
struct RasterContext;
} // namespace orendergraph

namespace omega
{

class CameraSystem;
class GraphicsSystem;

struct SkyboxRenderFeatureDesc
{
    Entity camera;
    CameraType camera_type;

    orendergraph::ResourceHandle<ogfx::Image> environment_map;
    orendergraph::ResourceHandle<ogfx::ImageView> environment_map_view;

    orendergraph::ResourceHandle<ogfx::Image> color_image;
    orendergraph::ResourceHandle<ogfx::ImageView> color_image_view;

    orendergraph::ResourceHandle<ogfx::Image> depth_image;
    orendergraph::ResourceHandle<ogfx::ImageView> depth_image_view;

    CameraSystem& cameras;
    GraphicsSystem& graphics;
};

class SkyboxRenderFeature : public orenderer::IRenderFeature
{
  public:
    explicit SkyboxRenderFeature(const SkyboxRenderFeatureDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;
    void on_swapchain_recreated(orenderer::RenderFeatureContext& ctx) override;

    void set_camera(Entity camera, CameraType camera_type);

  private:
    void create_resources(orendergraph::RenderGraph& rg);
    void add_node(orendergraph::RenderGraph& rg);

    SkyboxRenderFeatureDesc m_desc;

    orendergraph::ResourceHandle<ogfx::Sampler> m_sampler;

    orendergraph::RenderGraphNode<orendergraph::RasterContext>* m_skybox_node = nullptr;
};

} // namespace omega

#endif // OMEGA_SKYBOX_FEATURE_HPP