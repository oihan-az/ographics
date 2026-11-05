#ifndef ORENDERER_RENDER_PIPELINE_TPP
#define ORENDERER_RENDER_PIPELINE_TPP

namespace orenderer
{

template <typename TFeature, typename TDesc> 
TFeature& RenderPipeline::add_feature(const TDesc& desc)
{
    auto feature = std::make_unique<TFeature>(desc);

    TFeature& result = *feature;
    m_features.push_back(std::move(feature));

    RenderFeatureContext context(m_render_graph);
    m_features.back()->build(context);

    return result;
}

} // namespace orenderer

#endif