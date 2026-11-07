#ifndef OMEGA_EVENT_SYSTEM_HPP
#define OMEGA_EVENT_SYSTEM_HPP

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
    void swap_all_buffers();

    template <typename Event> void publish(Event event);
    template <typename Event> EventQueue<Event>& get_queue();

private:
    std::unordered_map<std::type_index, std::unique_ptr<IEventQueue>> m_queues;
};

} // namespace omega

#include "event_system.tpp"

#endif // OMEGA_EVENT_SYSTEM_HPP