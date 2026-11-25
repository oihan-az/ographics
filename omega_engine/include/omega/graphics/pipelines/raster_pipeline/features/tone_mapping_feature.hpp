#ifndef OMEGA_TONE_MAPPING_FEATURE_HPP
#define OMEGA_TONE_MAPPING_FEATURE_HPP

#include <orenderer/render_feature.hpp>
#include <orendergraph/resource_handle.hpp>

namespace ogfx
{
class Image;
class ImageView;
class Sampler;
class Buffer;
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

struct ToneMappingFeatureDesc
{
    orendergraph::ResourceHandle<ogfx::Image> input_image;
    orendergraph::ResourceHandle<ogfx::ImageView> input_image_view;

    orendergraph::ResourceHandle<ogfx::Buffer> exposure_buffer;

    GraphicsSystem& graphics;

    float exposure = 1.0f;
};

class ToneMappingFeature : public orenderer::IRenderFeature
{
  public:
    explicit ToneMappingFeature(const ToneMappingFeatureDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;
    void on_swapchain_recreated(orenderer::RenderFeatureContext& ctx) override;

    void set_exposure(float exposure);

    orendergraph::ResourceHandle<ogfx::Image> out_image() const
    {
        return m_out_image;
    }

    orendergraph::ResourceHandle<ogfx::ImageView> out_image_view() const
    {
        return m_out_image_view;
    }

  private:
    void create_resources(orendergraph::RenderGraph& rg);
    void create_out_resources(orendergraph::RenderGraph& rg);
    void add_node(orendergraph::RenderGraph& rg);

    ToneMappingFeatureDesc m_desc;

    orendergraph::ResourceHandle<ogfx::Image> m_out_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_out_image_view;
    orendergraph::ResourceHandle<ogfx::Sampler> m_sampler;

    orendergraph::RenderGraphNode<orendergraph::ComputeContext>* m_tone_mapping_node = nullptr;
};

} // namespace omega

#endif // OMEGA_TONE_MAPPING_FEATURE_HPP