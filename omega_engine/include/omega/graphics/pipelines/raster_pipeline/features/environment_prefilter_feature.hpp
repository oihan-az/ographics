#ifndef OMEGA_ENVIRONMENT_PREFILTER_FEATURE_HPP
#define OMEGA_ENVIRONMENT_PREFILTER_FEATURE_HPP

#include <orenderer/render_feature.hpp>

#include <orendergraph/resource_handle.hpp>

#include <cstdint>
#include <vector>

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
struct ComputeContext;
} // namespace orendergraph

namespace omega
{

class GraphicsSystem;

struct EnvironmentPrefilterFeatureDesc
{
    orendergraph::ResourceHandle<ogfx::Image> environment_map;
    orendergraph::ResourceHandle<ogfx::ImageView> environment_map_view;

    GraphicsSystem& graphics;

    uint32_t sample_count = 128;
};

class EnvironmentPrefilterFeature : public orenderer::IRenderFeature
{
  public:
    explicit EnvironmentPrefilterFeature(const EnvironmentPrefilterFeatureDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;

    orendergraph::ResourceHandle<ogfx::Image> out_image() const
    {
        return m_out_image;
    }

    orendergraph::ResourceHandle<ogfx::ImageView> out_image_view() const
    {
        return m_out_view;
    }

  private:
    void create_resources(orendergraph::RenderGraph& rg);
    void add_node(orendergraph::RenderGraph& rg);

    EnvironmentPrefilterFeatureDesc m_desc;

    uint32_t m_resolution = 0;
    uint32_t m_mip_levels = 0;

    orendergraph::ResourceHandle<ogfx::Image> m_out_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_out_view;
    std::vector<orendergraph::ResourceHandle<ogfx::ImageView>> m_storage_views;

    orendergraph::ResourceHandle<ogfx::Sampler> m_sampler;

    orendergraph::RenderGraphNode<orendergraph::ComputeContext>* m_prefilter_node = nullptr;
};

} // namespace omega

#endif // OMEGA_ENVIRONMENT_PREFILTER_FEATURE_HPP