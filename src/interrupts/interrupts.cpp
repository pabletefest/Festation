#include "interrupts.hpp"
#include <utility>

constexpr auto festation::InterruptsHandler::setInterruptSource(festation::InterruptSource source) -> void
{
    I_STAT.raw |= source;
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

auto festation::InterruptsHandler::write32(uint32_t address, uint32_t value) -> void
{
    switch (address)
    {
    case 0x1F801070:
        I_STAT.raw = value;
        break;
    case 0x1F801074:
        I_MASK.raw = value;
        break;
    default:
        std::unreachable();
    }
}
