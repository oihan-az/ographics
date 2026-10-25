#include "vk_pipeline.hpp"

#include "vk_device.hpp"

namespace ogfx
{

Pipeline::Pipeline(const Device& device) : m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;
}

Pipeline::~Pipeline()
{
    if (m_impl && m_impl->m_pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(m_impl->m_device, m_impl->m_pipeline, nullptr);
        m_impl->m_pipeline = VK_NULL_HANDLE;
    }
}

Pipeline::Pipeline(Pipeline&&) noexcept = default;

Pipeline& Pipeline::operator=(Pipeline&&) noexcept = default;

} // namespace ogfx