#include "interrupts.hpp"
#include <utility>

auto festation::InterruptsHandler::read16(uint32_t address) -> uint16_t
{
    uint16_t readValue = 0;

    switch (address)
    {
    case 0x1F801070:
        readValue = I_STAT.raw & 0xFFFF;
        break;
    case 0x1F801074:
        readValue = I_MASK.raw & 0xFFFF;
        break;
    default:
        std::unreachable();
    }

    return readValue;
}

auto festation::InterruptsHandler::read32(uint32_t address) -> uint32_t
{
    uint32_t readValue = 0;

    switch (address)
    {
    case 0x1F801070:
        readValue = I_STAT.raw;
        break;
    case 0x1F801074:
        readValue = I_MASK.raw;
        break;
    default:
        std::unreachable();
    }

    return readValue;
}

auto festation::InterruptsHandler::write16(uint32_t address, uint16_t value) -> void
{
    switch (address)
    {
    case 0x1F801070:
        I_STAT.raw &= (value & 0x7FF); // No effect when bit is 1
        break;
    case 0x1F801074:
        I_MASK.raw = (value & 0x7FF);
        break;
    default:
        std::unreachable();
    }
}

auto festation::InterruptsHandler::write32(uint32_t address, uint32_t value) -> void
{
    switch (address)
    {
    case 0x1F801070:
        I_STAT.raw &= (value & 0x7FF); // No effect when bit is 1
        break;
    case 0x1F801074:
        I_MASK.raw = (value & 0x7FF);
        break;
    default:
        std::unreachable();
    }
}

auto festation::InterruptsHandler::setInterruptSource(festation::InterruptSource source) -> void
{
    I_STAT.raw |= source;
}

auto festation::InterruptsHandler::clearInterruptSource(festation::InterruptSource source) -> void
{
    I_STAT.raw &= ~source;
}

auto festation::InterruptsHandler::isInterruptPending() const -> bool
{
    return I_STAT.raw & I_MASK.raw & 0x7FF;
}
