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
};

} // namespace orenderer

#endif // ORENDERER_RENDER_FEATURE_HPP