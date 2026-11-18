#ifndef ORENDERER_RENDER_FEATURE_HPP
#define ORENDERER_RENDER_FEATURE_HPP

namespace orenderer
{

class RenderFeatureContext;

class IRenderFeature
{
  public:
    virtual ~IRenderFeature() = default;

    virtual void build(RenderFeatureContext& ctx) = 0;
    virtual void on_swapchain_recreated(RenderFeatureContext& ctx) {};
};

} // namespace orenderer

#endif // ORENDERER_RENDER_FEATURE_HPP