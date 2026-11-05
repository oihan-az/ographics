#ifndef OMEGA_EVENT_QUEUE_TPP
#define OMEGA_EVENT_QUEUE_TPP

#include <algorithm>

namespace omega
{

template<typename Event>
void EventQueue<Event>::prepare_dispatch()
{
    // Snapshot pending events for this dispatch cycle
    dispatch_events.clear();
    dispatch_events.swap(pending_events);

    // Snapshot listeners so new subscriptions wait until next cycle
    dispatch_listener_ids.clear();

    for (const Listener& listener : listeners)
        dispatch_listener_ids.push_back(listener.id);
}

template <typename Event> 
void EventQueue<Event>::dispatch()
{
    for (const Event& event : dispatch_events)
    {
        for (uint64_t id : dispatch_listener_ids)
        {
            // Find the listener again because callbacks may modify the listener list
            auto it = std::find_if(listeners.begin(), listeners.end(),
                                   [id](const Listener& listener) { return listener.id == id; });

            // Skip listeners that were unsubscribed during dispatch
            if (it == listeners.end())
                continue;

            // Copy the callback so modifications to the listener list
            // cannot invalidate it while it is executing
            auto callback = it->callback;
            callback(event);
        }
    }

    // Release the prepared data after dispatch
    dispatch_events.clear();
    dispatch_listener_ids.clear();
}

template<typename Event> void EventQueue<Event>::unsubscribe(uint64_t subscription)
{
    std::erase_if(listeners, [subscription](const Listener& listener) { return listener.id == subscription; });
}

} // namespace omega

#endif // OMEGA_EVENT_QUEUE_TPP