#ifndef FRAME_EVENT_QUEUE_TPP
#define FRAME_EVENT_QUEUE_TPP

#include <omega/core/event/event_system.hpp>

namespace omega
{

template <typename Event>
FrameEventQueue<Event>::FrameEventQueue(uint32_t frames_in_flight) : m_frame_events(frames_in_flight)
{
}

template <typename Event>
void FrameEventQueue<Event>::capture(EventSystem& event_system)
{
    auto& events = event_system.get_queue<Event>().get_events();
    for (auto& frame_events : m_frame_events)
    {
        frame_events.insert(frame_events.end(), events.begin(), events.end());
    }
}

template <typename Event>
void FrameEventQueue<Event>::clear(uint32_t frame_index)
{
    m_frame_events[frame_index].clear();
}

template <typename Event>
const std::vector<Event>& FrameEventQueue<Event>::get_events(uint32_t frame_index) const
{
    return m_frame_events[frame_index];
}

} // namespace omega

#endif // FRAME_EVENT_QUEUE_TPP