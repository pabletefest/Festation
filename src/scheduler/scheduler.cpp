#include "scheduler.hpp"
#include "event_types.hpp"

auto festation::Scheduler::scheduleEvent(Event&& event) -> void
{
    event.time += m_globalTime;
    m_eventsQueue.push(event);
}

auto festation::Scheduler::step(uint64_t cycles) -> void
{
    m_globalTime += cycles;

    while (eventPending()) {
        dispatchNearestEvent();
    }
}

auto festation::Scheduler::eventPending() -> bool
{
    return !m_eventsQueue.empty() && m_globalTime >= m_eventsQueue.top().time;
}

auto festation::Scheduler::dispatchNearestEvent() -> void
{
    Event event = m_eventsQueue.top();
    m_eventsQueue.pop();
    event.callback();
}