#ifndef OMEGA_DITHERING_FEATURE_HPP
#define OMEGA_DITHERING_FEATURE_HPP

#include <orenderer/render_feature.hpp>

#include <orendergraph/resource_handle.hpp>

namespace ogfx
{
class Image;
class ImageView;
class CommandBuffer;
} // namespace ogfx

namespace orendergraph
{
class RenderGraph;
template <typename TContext> class RenderGraphNode;
struct ComputeContext;
} // namespace orendergraph

namespace omega
{

class GraphicsSystem;

struct DitheringFeatureDesc
{
    orendergraph::ResourceHandle<ogfx::Image> input_image;
    orendergraph::ResourceHandle<ogfx::ImageView> input_image_view;
    GraphicsSystem& graphics;
};

class DitheringFeature : public orenderer::IRenderFeature
{
  public:
    explicit DitheringFeature(const DitheringFeatureDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;
    void on_swapchain_recreated(orenderer::RenderFeatureContext& ctx) override;

    orendergraph::ResourceHandle<ogfx::Image> out_image() const
    {
        return m_out_image;
    }

  private:
    void create_resources(orendergraph::RenderGraph& rg);
    void add_node(orendergraph::RenderGraph& rg);

    DitheringFeatureDesc m_desc;

    orendergraph::ResourceHandle<ogfx::Image> m_out_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_out_view;

    orendergraph::RenderGraphNode<orendergraph::ComputeContext>* m_dithering_node = nullptr;
};

} // namespace omega

#endif // OMEGA_DITHERING_FEATURE_HPP