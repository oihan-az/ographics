#ifndef OMEGA_EDITOR_UI_FEATURE_HPP
#define OMEGA_EDITOR_UI_FEATURE_HPP

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

class IResourceAccess;
class RenderGraph;
template <typename TContext> class RenderGraphNode;
struct RasterContext;

} // namespace orendergraph

namespace omega
{

class GraphicsSystem;
class PlatformSystem;

} // namespace omega

namespace omega_editor
{

class ImGuiLayer;

struct UIFeatureDesc
{
    std::vector<orendergraph::ResourceHandle<ogfx::Image>> m_swapchain_images;

    ImGuiLayer& imgui_layer;
    omega::PlatformSystem& platform;
    omega::GraphicsSystem& graphics;
};

class UIFeature : public orenderer::IRenderFeature
{
  public:
    UIFeature(const UIFeatureDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;
    void on_swapchain_recreated(orenderer::RenderFeatureContext& ctx) override;

    orendergraph::RenderGraphNode<orendergraph::RasterContext>* GetNode() { return m_node; }

private:
    UIFeatureDesc m_desc;

    std::vector<orendergraph::IResourceAccess*> m_swapchain_reads;
    std::vector<orendergraph::IResourceAccess*> m_swapchain_writes;

    std::vector<orendergraph::ResourceHandle<ogfx::ImageView>> m_swapchain_image_views;

    orendergraph::RenderGraphNode<orendergraph::RasterContext>* m_node = nullptr;

    void add_node(orendergraph::RenderGraph& rg);
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_UI_FEATURE_HPP