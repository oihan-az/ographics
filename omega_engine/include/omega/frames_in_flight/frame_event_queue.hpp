#ifndef FRAME_EVENT_QUEUE_HPP
#define FRAME_EVENT_QUEUE_HPP

#include <cstdint>
#include <vector>

namespace omega
{

class EventSystem;

template <typename Event> 
class FrameEventQueue
{
  public:
    FrameEventQueue(uint32_t frames_in_flight);

    void capture(EventSystem& event_system);
    void clear(uint32_t frame_index);

    const std::vector<Event>& get_events(uint32_t frame_index) const;

  private:
    std::vector<std::vector<Event>> m_frame_events;
};

} // namespace omega

#include "frame_event_queue.tpp"

#endif // FRAME_EVENT_QUEUE_HPP