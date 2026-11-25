#ifndef OMEGA_BRDF_LUT_GEN_HPP
#define OMEGA_BRDF_LUT_GEN_HPP

#include <orenderer/render_feature.hpp>
#include <orendergraph/resource_handle.hpp>

namespace ogfx
{
class Image;
class ImageView;
} // namespace ogfx

namespace orendergraph
{
class RenderGraph;
template <typename TContext> class RenderGraphNode;
struct ComputeContext;
} // namespace orendergraph

namespace omega
{

struct BrdfLutGenDesc {};

class BrdfLutGen : public orenderer::IRenderFeature
{
  public:
    BrdfLutGen(const BrdfLutGenDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;

    orendergraph::ResourceHandle<ogfx::Image> brdf_lut() const
    {
        return m_brdf_lut;
    }

    orendergraph::ResourceHandle<ogfx::ImageView> brdf_lut_view() const
    {
        return m_brdf_lut_view;
    }

  private:
    void create_resources(orendergraph::RenderGraph& rg);
    void add_node(orendergraph::RenderGraph& rg);

    orendergraph::ResourceHandle<ogfx::Image> m_brdf_lut;
    orendergraph::ResourceHandle<ogfx::ImageView> m_brdf_lut_view;

    orendergraph::RenderGraphNode<orendergraph::ComputeContext>* m_brdf_lut_node = nullptr;
};

} // namespace omega

#endif // OMEGA_BRDF_LUT_GEN_HPP