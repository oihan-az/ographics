#ifndef ORENDERER_RENDERER_TPP
#define ORENDERER_RENDERER_TPP

#include <type_traits>
#include <utility>

namespace orenderer
{

template <typename TPipeline> 
TPipeline& Renderer::create_pipeline()
{
    auto pipeline = std::make_unique<TPipeline>(m_physical_device, m_device, m_resource_allocator, m_registry);
    TPipeline& result = *pipeline;

    m_pipelines.push_back(std::move(pipeline));
    return result;
}

template <typename TPipeline> 
TPipeline* Renderer::pipeline(uint32_t index)
{
    if (index >= m_pipelines.size())
        return nullptr;

    return static_cast<TPipeline*>(m_pipelines[index].get());
}

template<typename TPipeline, typename TDesc>
void Renderer::setup(TDesc& desc)
{
    if (!active_pipeline())
        throw std::runtime_error("No active render pipeline.");

    pipeline<TPipeline>(active_pipeline_index())->setup(desc);
}

} // namespace orenderer
#endif