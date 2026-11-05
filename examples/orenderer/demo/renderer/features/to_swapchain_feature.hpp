#ifndef TO_SWAPCHAIN_FEATURE_HPP
#define TO_SWAPCHAIN_FEATURE_HPP

#include <orenderer/render_feature.hpp>

#include <orendergraph/resource_handle.hpp>
#include <ogfx/image.hpp>

#include <vector>
 
namespace ogfx
{
class Swapchain;
}

namespace orendergraph
{
class RenderGraph;
}

struct ToSwapchainDesc
{
    orendergraph::ResourceHandle<ogfx::Image> image;

    std::vector<orendergraph::ResourceHandle<ogfx::Image>> swapchain_images;
    uint32_t swapchain_width = 0;
    uint32_t swapchain_height = 0;

    uint32_t* image_index = nullptr;
};

class ToSwapchain : public orenderer::IRenderFeature
{
  public:
    ToSwapchain(const ToSwapchainDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;

  private:
    ToSwapchainDesc m_desc;

    void add_blit_node(orendergraph::RenderGraph& rg);
};

#endif // DEFERRED_FEATURE_HPP