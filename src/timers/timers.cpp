#include "timers.hpp"
#include "interrupts/interrupts.hpp"
#include "scheduler/event_types.hpp"

#include <cstdint>
#include <utility>

festation::Timer::Timer(float baseClockFactor, InterruptsHandler& interruptsHandler, Scheduler& scheduler, PsxGpu& gpu)
    : m_baseClockFactor(baseClockFactor), m_interruptsHandler(interruptsHandler), m_scheduler(scheduler), m_gpu(gpu)
{
    
}

auto festation::Timer::read8(uint32_t address) -> uint8_t
{
    switch (address & 0xF)
    {
    case 0:
        return calculateCounterOnRead() & 0xFF;
    case 4:
    {
        uint32_t result = m_counterModeReg.raw & 0xFF;
        m_counterModeReg.reachedTarget = 0;
        m_counterModeReg.reachedMax = 0;
        return result;
    }
    case 8:
        return m_targetCounterReg.target & 0xFF;
    default:
        std::unreachable();
    }
}

auto festation::Timer::read16(uint32_t address) -> uint16_t
{
    switch (address & 0xF)
    {
    case 0:
        return calculateCounterOnRead();
    case 4:
    {
        uint32_t result = m_counterModeReg.raw & 0xFFFF;
        m_counterModeReg.reachedTarget = 0;
        m_counterModeReg.reachedMax = 0;
        return result;
    }
    case 8:
        return m_targetCounterReg.target;
    default:
        std::unreachable();
    }
}

auto festation::Timer::read32(uint32_t address) -> uint32_t
{
    switch (address & 0xF)
    {
    case 0:
        return calculateCounterOnRead() | (m_currentCounterReg.raw & 0xFFFF0000);
    case 4:
    {
        uint32_t result = m_counterModeReg.raw;
        m_counterModeReg.reachedTarget = 0;
        m_counterModeReg.reachedMax = 0;
        return result;
    }
    case 8:
        return m_targetCounterReg.raw;
    default:
        std::unreachable();
    }
}

auto festation::Timer::write8(uint32_t address, uint8_t value) -> void
{
    switch (address & 0xF)
    {
    case 0:
        m_currentCounterReg.current = value;
        scheduleTimerEvent();
        break;
    case 4:
        m_counterModeReg.raw = (m_counterModeReg.raw & 0xFFFFFF00) | value;
        processCounterModeChange();
        scheduleTimerEvent();
        break;
    case 8:
        m_targetCounterReg.target = value;
        break;
    default:
        std::unreachable();
    }
}

auto festation::Timer::write16(uint32_t address, uint16_t value) -> void
{
    switch (address & 0xF)
    {
    case 0:
        m_currentCounterReg.current = value;
        scheduleTimerEvent();
        break;
    case 4:
        m_counterModeReg.raw = (m_counterModeReg.raw & 0xFFFF0000) | value;
        processCounterModeChange();
        scheduleTimerEvent();
        break;
    case 8:
        m_targetCounterReg.target = value;
        break;
    default:
        std::unreachable();
    }
}

auto festation::Timer::write32(uint32_t address, uint32_t value) -> void
{
    switch (address & 0xF)
    {
    case 0:
        m_currentCounterReg.raw = value;
        scheduleTimerEvent();
        break;
    case 4:
        m_counterModeReg.raw = value;
        processCounterModeChange();
        scheduleTimerEvent();
        break;
    case 8:
        m_targetCounterReg.raw = value;
        break;
    default:
        std::unreachable();
    }
}

auto festation::Timer::scheduleTimerEvent() -> void
{
    m_scheduler.descheduleEvent(EventType::Timer);
    m_lastReadCycle = m_scheduler.getGlobalTime();

    if (m_counterModeReg.targetReachedIrq) {
        uint64_t cycles = (m_targetCounterReg.target - m_currentCounterReg.current) * m_clockFactor;

        m_scheduler.scheduleEvent({ EventType::Timer, cycles, [this](){
            m_counterModeReg.irqRequest = 0;
            m_currentCounterReg.current = m_targetCounterReg.target;
            raiseTimerIrq();
        } });
    }

    if (m_counterModeReg.maxReachedIrq) {
        uint64_t cycles = (0xFFFF - m_currentCounterReg.current) * m_clockFactor;

        m_scheduler.scheduleEvent({ EventType::Timer, cycles, [this](){
            m_counterModeReg.irqRequest = 0;
            m_currentCounterReg.current = 0xFFFF;
            raiseTimerIrq();
        } });
    }

    uint64_t cycles = (m_counterModeReg.whenResetCounter) ? (m_targetCounterReg.target - m_currentCounterReg.current) : (0xFFFF - m_currentCounterReg.current);

    m_scheduler.scheduleEvent({ EventType::Timer, cycles, [this](){
        m_currentCounterReg.current = 0;
    } });
}

auto festation::Timer::processCounterModeChange() -> void
{
    m_counterModeReg.irqRequest = 1;
    m_currentCounterReg.current = 0;
    setClockSource();
}

auto festation::Timer::calculateCounterOnRead() -> uint16_t
{
    return (m_scheduler.getGlobalTime() - m_lastReadCycle) / m_clockFactor;
}

festation::Timer0::Timer0(float baseClockFactor, InterruptsHandler& interruptsHandler, Scheduler& scheduler, PsxGpu& gpu)
    : Timer(baseClockFactor, interruptsHandler, scheduler, gpu)
{
    
}

auto festation::Timer0::setClockSource() -> void
{
    switch (m_counterModeReg.clockSrc)
    {
    case 0:
    case 2:
        m_clockFactor = 1;
        break;
    case 1:
    case 3:
        m_clockFactor = m_baseClockFactor * m_gpu.dotclockFactor();
        break;
    default:
        std::unreachable();
    }
}

auto festation::Timer0::raiseTimerIrq() -> void
{
    m_interruptsHandler.setInterruptSource(InterruptSource::Tmr0Src);
}

festation::Timer1::Timer1(float baseClockFactor, InterruptsHandler& interruptsHandler, Scheduler& scheduler, PsxGpu& gpu)
    : Timer(baseClockFactor, interruptsHandler, scheduler, gpu)
{

}

auto festation::Timer1::setClockSource() -> void
{
    switch (m_counterModeReg.clockSrc)
    {
    case 0:
    case 2:
        m_clockFactor = 1;
        break;
    case 1:
    case 3:
        m_clockFactor = m_baseClockFactor;
        break;
    default:
        std::unreachable();
    }
}

auto festation::Timer1::scheduleTimerEvent() -> void
{
    /** @brief Handled on the HBlank event when it's the clock source */
    if ((m_counterModeReg.clockSrc & 1) == 0) {
        Timer::scheduleTimerEvent();
    }
}

auto festation::Timer1::raiseTimerIrq() -> void
{
    m_interruptsHandler.setInterruptSource(InterruptSource::Tmr1Src);
}

auto festation::Timer1::onHBlankTimer1Update() -> void
{
    if (m_counterModeReg.clockSrc & 1) {
        m_currentCounterReg.current++;

        if (m_currentCounterReg.current == m_targetCounterReg.target) {
            if (m_counterModeReg.whenResetCounter == 1) {
                m_currentCounterReg.current = 0;
            }

            if (m_counterModeReg.targetReachedIrq) {
                m_counterModeReg.irqRequest = 0;
                raiseTimerIrq();
            }
        }
        
        if (m_currentCounterReg.current == 0xFFFF) {
            if (m_counterModeReg.whenResetCounter == 0) {
                m_currentCounterReg.current = 0;
            }

            if (m_counterModeReg.maxReachedIrq) {
                m_counterModeReg.irqRequest = 0;
                raiseTimerIrq();
            }
        }
    }
}

festation::Timer2::Timer2(float baseClockFactor, InterruptsHandler& interruptsHandler, Scheduler& scheduler, PsxGpu& gpu)
    : Timer(baseClockFactor, interruptsHandler, scheduler, gpu)
{
    
}

auto festation::Timer2::setClockSource() -> void
{
    switch (m_counterModeReg.clockSrc)
    {
    case 0:
    case 1:
        m_clockFactor = 1;
        break;
    case 2:
    case 3:
        m_clockFactor = m_baseClockFactor;
        break;
    default:
        std::unreachable();
    }
}

auto festation::Timer2::raiseTimerIrq() -> void
{
    m_interruptsHandler.setInterruptSource(InterruptSource::Tmr2Src);
}
