#pragma once

#include "event_types.hpp"

#include <cstdint>
#include <queue>

namespace festation {
    class Scheduler {
        /** @brief Greater-comparison performed to achieve a min-heap */
        using EventsQueue = std::priority_queue<Event, std::vector<Event>, std::greater<Event>>;

    public:
        auto scheduleEvent(Event&& event) -> void;
        auto step(uint64_t cycles) -> void;
        auto advanceFor(uint64_t cycles) -> void;
        auto nextEventTime() -> uint64_t;
        auto dispatchPastEvents() -> void;

    private:
        auto eventPending() const -> bool;
        auto dispatchNearestEvent() -> void;

    private:
        EventsQueue m_eventsQueue{};
        uint64_t m_globalTime{};
    };
};
