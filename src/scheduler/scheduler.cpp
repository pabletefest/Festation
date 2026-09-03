#include "scheduler.hpp"
#include "event_types.hpp"
#include <algorithm>

auto festation::Scheduler::scheduleEvent(Event&& event) -> void
{
    event.time += m_globalTime;
    m_eventsQueue.insert(event);
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
    return m_eventsQueue.cbegin()->time - m_globalTime;
}

auto festation::Scheduler::dispatchPastEvents() -> void
{
    while (eventPending()) {
        dispatchNearestEvent();
    }
}

auto festation::Scheduler::getGlobalTime() const -> uint64_t
{
    return m_globalTime;
}

auto festation::Scheduler::descheduleEvent(EventType type) -> void
{
    auto predicate = [type](const Event& ev) -> bool {
        return ev.type == type;
    };

    auto eventIt = std::find_if(m_eventsQueue.begin(), m_eventsQueue.end(), predicate);

    while (eventIt != m_eventsQueue.end()) {
        m_eventsQueue.erase(eventIt);

        eventIt = std::find_if(m_eventsQueue.begin(), m_eventsQueue.end(), predicate);
    }
}

auto festation::Scheduler::eventPending() const -> bool
{
    return !m_eventsQueue.empty() && m_globalTime >= m_eventsQueue.cbegin()->time;
}

auto festation::Scheduler::dispatchNearestEvent() -> void
{
    auto event = m_eventsQueue.extract(m_eventsQueue.cbegin());
    event.value().callback();
}