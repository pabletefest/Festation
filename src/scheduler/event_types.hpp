#pragma once

#include <cstdint>
#include <functional>

namespace festation {
    enum class EventType {
        VBlank,
        CdromInt1,
        CdromInt2,
        CdromInt3,
        CdromInt4,
        CdromInt5,
        DmaInt,
        Timer0Int,
        Timer1Int,
        Timer2Int,
    };

    struct Event {
        EventType type;
        uint64_t time;
        std::function<void(void)> callback;

        bool operator<(const Event& other) const {
            return this->time < other.time;
        }

        bool operator>(const Event& other) const {
            return this->time > other.time;
        }
    };
};
