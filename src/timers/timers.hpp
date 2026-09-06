#pragma once

#include "gpu/gpu.hpp"
#include "interrupts/interrupts.hpp"
#include "scheduler/scheduler.hpp"

namespace festation {
    class Timer {
    public:
        Timer(float baseClockFactor, InterruptsHandler& interruptsHandler, Scheduler& scheduler, PsxGpu& gpu);
        virtual ~Timer() = default;

        auto read8(uint32_t address) -> uint8_t;
        auto read16(uint32_t address) -> uint16_t;
        auto read32(uint32_t address) -> uint32_t;
        auto write8(uint32_t address, uint8_t value) -> void;
        auto write16(uint32_t address, uint16_t value) -> void;
        auto write32(uint32_t address, uint32_t value) -> void;

    protected:
        virtual auto setClockSource() -> void = 0;
        virtual auto scheduleTimerEvent() -> void;
        virtual auto raiseTimerIrq() -> void = 0;

    private:
        auto processCounterModeChange() -> void;
        auto calculateCounterOnRead() -> uint16_t;

    protected:
        const float m_baseClockFactor{};
        float m_clockFactor{};
        uint64_t m_lastReadCycle{};
        InterruptsHandler& m_interruptsHandler;
        Scheduler& m_scheduler;
        PsxGpu& m_gpu;

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

    class Timer0 : public Timer {
    public:
        Timer0(float baseClockFactor, InterruptsHandler& interruptsHandler, Scheduler& scheduler, PsxGpu& gpu);
        virtual ~Timer0() = default;

    private:
        auto setClockSource() -> void override;
        auto raiseTimerIrq() -> void override;
    };

    class Timer1 : public Timer {
    public:
        Timer1(float baseClockFactor, InterruptsHandler& interruptsHandler, Scheduler& scheduler, PsxGpu& gpu);
        virtual ~Timer1() = default;

        auto onHBlankTimer1Update() -> void;

    private:
        auto setClockSource() -> void override;
        auto scheduleTimerEvent() -> void override;
        auto raiseTimerIrq() -> void override;
    };

    class Timer2 : public Timer {
    public:
        Timer2(float baseClockFactor, InterruptsHandler& interruptsHandler, Scheduler& scheduler, PsxGpu& gpu);
        virtual ~Timer2() = default;

    private:
        auto setClockSource() -> void override;
        auto raiseTimerIrq() -> void override;
    };
};
