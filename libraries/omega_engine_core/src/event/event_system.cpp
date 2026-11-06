#include <omega/core/event/event_system.hpp>

#include <algorithm>

namespace omega
{

void EventSystem::unsubscribe(uint64_t subscription)
{
    for (auto& [type, queue] : m_queues)
        queue->unsubscribe(subscription);
}

void EventSystem::dispatch()
{
    // Prevent callbacks from recursively dispatching events
    if (m_is_dispatching)
        return;

    m_is_dispatching = true;

    try
    {
        // Snapshot existing queues so newly created queues
        // are deferred until the next dispatch cycle
        std::vector<IEventQueue*> queues;
        for (auto& [type, queue] : m_queues)
            queues.push_back(queue.get());

        // Prepare all queues before invoking any callbacks
        // This ensures events published during dispatch,
        // regardless of type, wait until the next cycle
        for (IEventQueue* queue : queues)
            queue->prepare_dispatch();

        // Dispatch the prepared events.
        for (IEventQueue* queue : queues)
            queue->dispatch();
    }
    catch (...)
    {
        m_is_dispatching = false;
        throw;
    }

    m_is_dispatching = false;
}

} // namespace omega