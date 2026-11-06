#ifndef OMEGA_EVENT_TPP
#define OMEGA_EVENT_TPP

#include <utility>

namespace omega
{

template<typename Event, typename Callback>
uint64_t EventSystem::subscribe(Callback&& callback)
{
    uint64_t id = m_next_subscription++;
    get_queue<Event>().listeners.push_back({ id, std::forward<Callback>(callback) });
    return id;
}

template<typename Event>
void EventSystem::publish(Event event)
{
    get_queue<Event>().pending_events.push_back(std::move(event));
}

template<typename Event>
EventQueue<Event>& EventSystem::get_queue()
{
    std::type_index type = typeid(Event);

    auto it = m_queues.find(type);

    if (it != m_queues.end())
        return *static_cast<EventQueue<Event>*>(it->second.get());

    auto queue = std::make_unique<EventQueue<Event>>();
    EventQueue<Event>& reference = *queue;

    m_queues.emplace(type, std::move(queue));

    return reference;
}

} // namespace omega

#endif // OMEGA_EVENT_TPP