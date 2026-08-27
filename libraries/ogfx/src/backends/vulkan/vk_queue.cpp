#include <ogfx/queue.hpp>

#include "vk_queue.hpp"

namespace ogfx
{

Queue::Queue() : m_impl(std::make_unique<Impl>()) {}

Queue::~Queue() {}

Queue::Queue(Queue&&) noexcept = default;

Queue& Queue::operator=(Queue&&) noexcept = default;

bool Queue::valid() const
{
    return m_impl && m_impl->m_queue != VK_NULL_HANDLE;
}

} // namespace ogfx