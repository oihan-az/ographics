
#ifndef OMEGA_AUTO_EXPOSURE_FEATURE_HPP
#define OMEGA_AUTO_EXPOSURE_FEATURE_HPP

#include <orenderer/render_feature.hpp>
#include <orendergraph/resource_handle.hpp>

#include <cstdint>

namespace ogfx
{
class Image;
class ImageView;
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

struct AutoExposureFeatureDesc
{
    orendergraph::ResourceHandle<ogfx::Image> input_image;
    orendergraph::ResourceHandle<ogfx::ImageView> input_image_view;

    float min_log_luminance = -12.0f;
    float max_log_luminance = 12.0f;

    float low_percentile = 0.40f;
    float high_percentile = 0.98f;
    float highlight_weight = 1.0f;

    float middle_gray = 0.18f;
    float min_exposure = 0.03f;
    float max_exposure = 12.0f;

    float speed_up = 1.5f;
    float speed_down = 0.5f;
};

class AutoExposureFeature : public orenderer::IRenderFeature
{
  public:
    explicit AutoExposureFeature(const AutoExposureFeatureDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;
    void on_swapchain_recreated(orenderer::RenderFeatureContext& ctx) override;

    void set_delta_time(float delta_time);

    orendergraph::ResourceHandle<ogfx::Buffer> exposure_buffer() const
    {
        return m_exposure_buffer;
    }

  private:
    void create_resources(orendergraph::RenderGraph& rg);
    void add_histogram_node(orendergraph::RenderGraph& rg);
    void add_exposure_node(orendergraph::RenderGraph& rg);

    AutoExposureFeatureDesc m_desc;

    orendergraph::ResourceHandle<ogfx::Buffer> m_histogram_buffer;
    orendergraph::ResourceHandle<ogfx::Buffer> m_exposure_buffer;

    orendergraph::RenderGraphNode<orendergraph::ComputeContext>* m_histogram_node = nullptr;
    orendergraph::RenderGraphNode<orendergraph::ComputeContext>* m_exposure_node = nullptr;

    float m_delta_time = 1.0f / 60.0f;
    bool m_has_exposure_history = false;
};

} // namespace omega

#endif // OMEGA_AUTO_EXPOSURE_FEATURE_HPP