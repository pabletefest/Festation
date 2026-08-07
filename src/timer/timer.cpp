#include "timer.hpp"

#include <utility>

festation::Timer::Timer(Scheduler& scheduler)
    : m_scheduler(scheduler)
{
    
}

auto festation::Timer::read8(uint32_t address) -> uint8_t
{
    switch (address & 0xF)
    {
    case 0:
        return m_currentCounterValue.raw & 0xFF;
    case 4:
        return m_counterMode.raw & 0xFF;
    case 8:
        return m_counterTargetValue.raw & 0xFF;
    default:
        std::unreachable();
    }
}

auto festation::Timer::read16(uint32_t address) -> uint16_t
{
    switch (address & 0xF)
    {
    case 0:
        return m_currentCounterValue.raw & 0xFFFF;
    case 4:
        return m_counterMode.raw & 0xFFFF;
    case 8:
        return m_counterTargetValue.raw & 0xFFFF;
    default:
        std::unreachable();
    }
}

auto festation::Timer::read32(uint32_t address) -> uint32_t
{
    switch (address & 0xF)
    {
    case 0:
        return m_currentCounterValue.raw;
    case 4:
        return m_counterMode.raw;
    case 8:
        return m_counterTargetValue.raw;
    default:
        std::unreachable();
    }
}

auto festation::Timer::write8(uint32_t address, uint8_t value) -> void
{
    switch (address & 0xF)
    {
    case 0:
        m_currentCounterValue.current = value;
        break;
    case 4:
        m_counterMode.raw = (m_counterMode.raw & 0xFFFFFF00) | value;
        break;
    case 8:
        m_counterTargetValue.target = value;
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
        m_currentCounterValue.current = value;
        break;
    case 4:
        m_counterMode.raw = (m_counterMode.raw & 0xFFFF0000) | value;
        break;
    case 8:
        m_counterTargetValue.target = value;
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
        m_currentCounterValue.raw = value;
        break;
    case 4:
        m_counterMode.raw = value;
        break;
    case 8:
        m_counterTargetValue.raw = value;
        break;
    default:
        std::unreachable();
    }
}
