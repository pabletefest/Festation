#pragma once

#include "interrupts/interrupts.hpp"
#include "scheduler/scheduler.hpp"

namespace festation {
    class Timer {
    public:
        Timer(InterruptsHandler& interruptsHandler, Scheduler& scheduler);

        auto read8(uint32_t address) -> uint8_t;
        auto read16(uint32_t address) -> uint16_t;
        auto read32(uint32_t address) -> uint32_t;
        auto write8(uint32_t address, uint8_t value) -> void;
        auto write16(uint32_t address, uint16_t value) -> void;
        auto write32(uint32_t address, uint32_t value) -> void;

    private:
        auto processCounterModeChange() -> void;

    private:
        InterruptsHandler& m_interruptsHandler;
        Scheduler& m_scheduler;
        float clockDivisor{};

        union {
            struct {
                uint32_t current : 16;
                uint32_t garbage : 16;
            };

            uint32_t raw;
        } m_currentCounterReg{};

        union {
            struct {
                uint32_t syncEnabled : 1;
                uint32_t syncMode : 2;
                uint32_t whenResetCounter : 1;
                uint32_t targetReachedIrq : 1;
                uint32_t maxReachedIrq : 1;
                uint32_t irqOnceRepeatMode : 1;
                uint32_t irqPulseToggleMode : 1;
                uint32_t clockSrc : 2;
                uint32_t irqRequest : 1;
                uint32_t reachedTarget : 1;
                uint32_t reachedMax : 1;
                uint32_t unknown : 3;
                uint32_t garbage : 16;
            };

            uint32_t raw;
        } m_counterModeReg{};

        union {
            struct {
                uint32_t target : 16;
                uint32_t garbage : 16;
            };

            uint32_t raw;
        } m_targetCounterReg{};
    };
};
