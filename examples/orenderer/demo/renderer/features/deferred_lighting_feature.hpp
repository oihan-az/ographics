#ifndef DEFERRED_FEATURE_HPP
#define DEFERRED_FEATURE_HPP

#include <orenderer/render_feature.hpp>

#include <orendergraph/resource_handle.hpp>
#include <ogfx/buffer.hpp>
#include <ogfx/image.hpp>

#include "../../scene/geometry_generation.hpp"

namespace orendergraph
{
class RenderGraph;
}

struct SceneGeometry;
struct DirectionalLight;

struct DeferredLightingDesc
{
    orendergraph::ResourceHandle<ogfx::Buffer> vertex_buffer;
    orendergraph::ResourceHandle<ogfx::Buffer> index_buffer;

    uint32_t swapchain_width = 0;
    uint32_t swapchain_height = 0;

    const std::vector<MeshCluster>* clusters = nullptr;
};

class DeferredLighting : public orenderer::IRenderFeature
{
  public:
    DeferredLighting(const DeferredLightingDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;

    const orendergraph::ResourceHandle<ogfx::Image> lighting_image() const { return m_lighting_image; };

  private:
    DeferredLightingDesc m_desc;

    orendergraph::ResourceHandle<ogfx::Image> m_albedo_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_albedo_image_view;

    orendergraph::ResourceHandle<ogfx::Image> m_normal_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_normal_image_view;

    orendergraph::ResourceHandle<ogfx::Image> m_depth_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_depth_image_view;

    orendergraph::ResourceHandle<ogfx::Image> m_lighting_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_lighting_image_view;

    void create_resources(orendergraph::RenderGraph& rg);
    void add_geometry_node(orendergraph::RenderGraph& rg);
    void add_lighting_node(orendergraph::RenderGraph& rg);
};

#endif // DEFERRED_FEATURE_HPP