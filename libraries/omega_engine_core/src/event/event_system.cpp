#include <omega/core/event/event_system.hpp>

namespace omega
{

void EventSystem::swap_all_buffers()
{
    for (auto& [type, queue] : m_queues)
    {
        queue->swap_buffers();
    }
}

} // namespace omega