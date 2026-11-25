#ifndef OMEGA_IRRADIANCE_MAP_GEN_HPP
#define OMEGA_IRRADIANCE_MAP_GEN_HPP

#include <orenderer/render_feature.hpp>
#include <orendergraph/resource_handle.hpp>

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

struct IrradianceMapGenDesc
{
    orendergraph::ResourceHandle<ogfx::Image> environment_map;
    orendergraph::ResourceHandle<ogfx::ImageView> environment_map_view;
};

class IrradianceMapGen : public orenderer::IRenderFeature
{
  public:
    explicit IrradianceMapGen(const IrradianceMapGenDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;

    orendergraph::ResourceHandle<ogfx::Image> irradiance_map() const
    {
        return m_irradiance_map;
    }

    orendergraph::ResourceHandle<ogfx::ImageView> irradiance_map_view() const
    {
        return m_irradiance_map_view;
    }

  private:
    void create_resources(orendergraph::RenderGraph& rg);
    void add_node(orendergraph::RenderGraph& rg);

    IrradianceMapGenDesc m_desc;

    orendergraph::ResourceHandle<ogfx::Image> m_irradiance_map;
    orendergraph::ResourceHandle<ogfx::ImageView> m_irradiance_map_view;
    orendergraph::ResourceHandle<ogfx::ImageView> m_irradiance_storage_view;
    orendergraph::ResourceHandle<ogfx::Sampler> m_environment_sampler;

    orendergraph::RenderGraphNode<orendergraph::ComputeContext>* m_irradiance_node = nullptr;
};

} // namespace omega

#endif // OMEGA_IRRADIANCE_MAP_GEN_HPP