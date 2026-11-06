#ifndef OMEGA_EVENT_HPP
#define OMEGA_EVENT_HPP

#include <omega/core/event/event_queue.hpp>

#include <cstdint>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace omega
{

class EventSystem
{
  public:
    template <typename Event, typename Callback> uint64_t subscribe(Callback&& callback);
    void unsubscribe(uint64_t subscription);

    template <typename Event> void publish(Event event);
    void dispatch();

    template <typename Event> EventQueue<Event>& get_queue();

private:
    std::unordered_map<std::type_index, std::unique_ptr<IEventQueue>> m_queues;
    uint64_t m_next_subscription = 1;

    bool m_is_dispatching = false;
};

} // namespace omega

#include "event.tpp"

#endif // OMEGA_EVENT_HPP