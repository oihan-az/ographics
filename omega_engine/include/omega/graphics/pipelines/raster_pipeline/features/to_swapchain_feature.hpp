#ifndef TO_SWAPCHAIN_FEATURE_HPP
#define TO_SWAPCHAIN_FEATURE_HPP

#include <orenderer/render_feature.hpp>

#include <orendergraph/resource_handle.hpp>

#include <vector>
#include <optional>
 
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
struct ClearContext;

} // namespace orendergraph

namespace omega
{

class GraphicsSystem;

struct RenderRegion
{
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

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

    void set_destination_region(const RenderRegion& region);
    void clear_destination_region();


  private:
    ToSwapchainDesc m_desc;

    std::optional<RenderRegion> m_destination_region;

    std::vector<orendergraph::ResourceHandle<ogfx::Image>> m_swapchain_images;

    orendergraph::RenderGraphNode<orendergraph::BlitContext>* m_to_swapchain_node = nullptr;
    orendergraph::RenderGraphNode<orendergraph::ClearContext>* m_clear_node = nullptr;

    void add_clear_node(orendergraph::RenderGraph& rg);
    void add_blit_node(orendergraph::RenderGraph& rg);
};

} // namespace omega

#endif // TO_SWAPCHAIN_FEATURE_HPP