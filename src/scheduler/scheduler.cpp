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

    dispatchPastEvents();
}

auto festation::Scheduler::advanceFor(uint64_t cycles) -> void
{
    m_globalTime += cycles;
}

auto festation::Scheduler::nextEventTime() -> uint64_t
{
    return m_eventsQueue.top().time - m_globalTime;
}

auto festation::Scheduler::dispatchPastEvents() -> void
{
    while (eventPending()) {
        dispatchNearestEvent();
    }
}

auto festation::Scheduler::eventPending() const -> bool
{
    return !m_eventsQueue.empty() && m_globalTime >= m_eventsQueue.top().time;
}

auto festation::Scheduler::dispatchNearestEvent() -> void
{
    Event event = m_eventsQueue.top();
    m_eventsQueue.pop();
    event.callback();
}