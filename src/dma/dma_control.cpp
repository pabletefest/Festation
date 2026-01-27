#include <dma/dma_control.hpp>
#include "dma_control.hpp"

festation::DmaControl::DmaControl()
{
    reset();
}

festation::DmaControl::~DmaControl()
{
}

void festation::DmaControl::reset()
{
    DPCR.raw = 0x07654321u;
    DICR.raw = 0;
}

uint32_t festation::DmaControl::read32(uint32_t address)
{
    switch(address)
    {
    case 0x1F8010F0:
        return DPCR.raw;
        break;
    case 0x1F8010F4:
        return DICR.raw;
        break;
    default:
        size_t channelId = ((address >> 4) & 0xFu) - 8u;
        return channels[channelId].read32(address);
    }

    return 0;
}

void festation::DmaControl::write32(uint32_t address, uint32_t value)
{
    switch(address)
    {
    case 0x1F8010F0:
        DPCR.raw = value;
        break;
    case 0x1F8010F4:
        DICR.raw = value;
        break;
    default:
        size_t channelId = ((address >> 4) & 0xFu) - 8u;
        channels[channelId].write32(address, value);
    }
}
