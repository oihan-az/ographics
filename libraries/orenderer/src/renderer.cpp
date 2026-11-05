#include <orenderer/renderer.hpp>

namespace orenderer
{

Renderer::Renderer(ogfx::PhysicalDevice& physical_device, ogfx::Device& device,
                   ogfx::ResourceAllocator& resource_allocator)
    : m_physical_device(physical_device), m_device(device), m_resource_allocator(resource_allocator)
{
}

Renderer::~Renderer() = default;

bool Renderer::set_active_pipeline(uint32_t index)
{
    if (index >= pipeline_count())
        return false;

    m_active_pipeline_index = index;
    return true;
}

bool Renderer::has_active_pipeline() const
{
    return m_active_pipeline_index != INVALID_PIPELINE_INDEX;
}

std::size_t Renderer::active_pipeline_index() const
{
    return m_active_pipeline_index;
}

RenderPipeline* Renderer::active_pipeline()
{
    if (m_active_pipeline_index == INVALID_PIPELINE_INDEX)
        return nullptr;

    return m_pipelines[m_active_pipeline_index].get();
}

const RenderPipeline* Renderer::active_pipeline() const
{
    if (m_active_pipeline_index == INVALID_PIPELINE_INDEX)
        return nullptr;

    return m_pipelines[m_active_pipeline_index].get();
}

RenderPipeline* Renderer::pipeline(uint32_t index)
{
    if (index >= m_pipelines.size())
        return nullptr;

    return m_pipelines[index].get();
}

const RenderPipeline* Renderer::pipeline(uint32_t index) const
{
    if (index >= m_pipelines.size())
        return nullptr;

    return m_pipelines[index].get();
}

uint32_t Renderer::pipeline_count() const
{
    return static_cast<uint32_t>(m_pipelines.size());
}

orendergraph::ResourcePoolRegistry& Renderer::resource_registry()
{
    return m_registry;
}

void Renderer::compile()
{
    auto* pipeline = active_pipeline();
    if (!pipeline)
        throw std::runtime_error("No active render pipeline.");

    pipeline->compile();
}

void Renderer::execute(ogfx::CommandBuffer& command_buffer)
{
    auto* pipeline = active_pipeline();
    if (!pipeline)
        throw std::runtime_error("No active render pipeline.");

    pipeline->execute(command_buffer);
}

} // namespace orenderer