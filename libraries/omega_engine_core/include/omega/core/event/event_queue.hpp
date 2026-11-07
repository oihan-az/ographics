#ifndef OMEGA_EVENT_QUEUE_HPP
#define OMEGA_EVENT_QUEUE_HPP

#include <cstdint>
#include <vector>

namespace omega
{

class IEventQueue
{
  public:
    virtual ~IEventQueue() = default;
    virtual void swap_buffers() = 0;
};

template <typename Event> 
class EventQueue : public IEventQueue
{
  public:
    void swap_buffers() override;

    void push(Event&& event);
    const std::vector<Event>& get_events() const;

  private:
    std::vector<Event> buffer_incoming;
    std::vector<Event> buffer_processing;
};

} // namespace omega

#include "event_queue.tpp"

#endif // OMEGA_EVENT_QUEUE_HPP