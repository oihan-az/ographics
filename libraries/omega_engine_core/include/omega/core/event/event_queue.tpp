#ifndef OMEGA_EVENT_QUEUE_TPP
#define OMEGA_EVENT_QUEUE_TPP

#include <algorithm>

namespace omega
{

template <typename Event> 
void EventQueue<Event>::swap_buffers()
{
    buffer_processing.clear();
    std::swap(buffer_incoming, buffer_processing);
}

template<typename Event> 
void EventQueue<Event>::push(Event&& event)
{
    buffer_incoming.push_back(std::forward<Event>(event));
}

template<typename Event> 
const std::vector<Event>& EventQueue<Event>::get_events() const
{
    return buffer_processing;
}

} // namespace omega

#endif // OMEGA_EVENT_QUEUE_TPP