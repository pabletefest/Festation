#pragma once

#include "event_types.hpp"

#include <cstdint>
#include <set>

namespace festation {
    class Scheduler {
        using EventsQueue = std::multiset<Event>;

    public:
        auto scheduleEvent(Event&& event) -> void;
        auto step(uint64_t cycles) -> void;
        auto advanceFor(uint64_t cycles) -> void;
        auto nextEventTime() -> uint64_t;
        auto dispatchPastEvents() -> void;
        auto getGlobalTime() const -> uint64_t;
        auto descheduleEvent(EventType type) -> void;

    private:
        auto eventPending() const -> bool;
        auto dispatchNearestEvent() -> void;

    private:
        EventsQueue m_eventsQueue{};
        uint64_t m_globalTime{};
    };
};
