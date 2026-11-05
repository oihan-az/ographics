#ifndef OMEGA_EVENT_QUEUE_HPP
#define OMEGA_EVENT_QUEUE_HPP

#include <cstdint>
#include <functional>
#include <vector>

namespace omega
{

struct IEventQueue
{
    virtual ~IEventQueue() = default;
    virtual void prepare_dispatch() = 0;
    virtual void dispatch() = 0;
    virtual void unsubscribe(uint64_t subscription) = 0;
};

template <typename Event> 
struct EventQueue : IEventQueue
{
    struct Listener
    {
        uint64_t id;
        std::function<void(const Event&)> callback;
    };

    std::vector<Listener> listeners;
    std::vector<Event> pending_events;

    std::vector<Event> dispatch_events;
    std::vector<uint64_t> dispatch_listener_ids;

    void prepare_dispatch() override;
    void dispatch() override;
    void unsubscribe(uint64_t subscription) override;
};

} // namespace omega

#include "event_queue.tpp"

#endif // OMEGA_EVENT_QUEUE_HPP