#ifndef TO_SWAPCHAIN_FEATURE_HPP
#define TO_SWAPCHAIN_FEATURE_HPP

#include <orenderer/render_feature.hpp>

#include <orendergraph/resource_handle.hpp>

#include <vector>
 
namespace ogfx
{

class Image;
class ImageView;

} // namespace ogfx

namespace orendergraph
{

class RenderGraph;
template <typename TContext> class RenderGraphNode;
struct BlitContext;

} // namespace orendergraph

namespace omega
{

class GraphicsSystem;

struct ToSwapchainDesc
{
    orendergraph::ResourceHandle<ogfx::Image> image;

    GraphicsSystem& graphics;
};

class ToSwapchain : public orenderer::IRenderFeature
{
  public:
    ToSwapchain(const ToSwapchainDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;
    void on_swapchain_recreated(orenderer::RenderFeatureContext& ctx) override;

    const std::vector<orendergraph::ResourceHandle<ogfx::Image>>& swapchain_images();

  private:
    ToSwapchainDesc m_desc;

    std::vector<orendergraph::ResourceHandle<ogfx::Image>> m_swapchain_images;

    orendergraph::RenderGraphNode<orendergraph::BlitContext>* m_to_swapchain_node = nullptr;

    void add_blit_node(orendergraph::RenderGraph& rg);
};

} // namespace omega

#endif // TO_SWAPCHAIN_FEATURE_HPP