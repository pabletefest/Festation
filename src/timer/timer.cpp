#include "timer.hpp"

#include <utility>

festation::Timer::Timer(InterruptsHandler& interruptsHandler, Scheduler& scheduler)
    : m_interruptsHandler(interruptsHandler), m_scheduler(scheduler)
{
    
}

auto festation::Timer::read8(uint32_t address) -> uint8_t
{
    switch (address & 0xF)
    {
    case 0:
        return m_currentCounterReg.raw & 0xFF;
    case 4:
    {
        uint32_t result = m_counterModeReg.raw & 0xFF;
        m_counterModeReg.reachedTarget = 0;
        m_counterModeReg.reachedMax = 0;
        return result;
    }
    case 8:
        return m_targetCounterReg.raw & 0xFF;
    default:
        std::unreachable();
    }
}

auto festation::Timer::read16(uint32_t address) -> uint16_t
{
    switch (address & 0xF)
    {
    case 0:
        return 0xFFFF;
    case 4:
    {
        uint32_t result = m_counterModeReg.raw & 0xFFFF;
        m_counterModeReg.reachedTarget = 0;
        m_counterModeReg.reachedMax = 0;
        return result;
    }
    case 8:
        return m_targetCounterReg.raw & 0xFFFF;
    default:
        std::unreachable();
    }
}

auto festation::Timer::read32(uint32_t address) -> uint32_t
{
    switch (address & 0xF)
    {
    case 0:
        return 0xFFFFFFFF;
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
        break;
    case 4:
        m_counterModeReg.raw = (m_counterModeReg.raw & 0xFFFFFF00) | value;
        processCounterModeChange();
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
        break;
    case 4:
        m_counterModeReg.raw = (m_counterModeReg.raw & 0xFFFF0000) | value;
        processCounterModeChange();
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
        break;
    case 4:
        m_counterModeReg.raw = value;
        processCounterModeChange();
        break;
    case 8:
        m_targetCounterReg.raw = value;
        break;
    default:
        std::unreachable();
    }
}

auto festation::Timer::processCounterModeChange() -> void
{
    m_counterModeReg.irqRequest = 1;
    m_currentCounterReg.current = 0;
    
}
