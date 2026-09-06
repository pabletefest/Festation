#pragma once

#include <cstdint>
#include <functional>

namespace festation {
    enum class EventType {
        VBlankStart,
        VBlankEnd,
        HBlank,
        CdromInt1,
        CdromInt2,
        CdromInt3,
        CdromInt4,
        CdromInt5,
        Dma0Int,
        Dma1Int,
        Dma2Int,
        Dma3Int,
        Dma4Int,
        Dma5Int,
        Dma6Int,
        Timer,
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
