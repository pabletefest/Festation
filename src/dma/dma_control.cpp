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
    DPCR.raw = 0x07654321;
    DICR.raw = 0;
}

uint32_t festation::DmaControl::read32(uint32_t address)
{
    return 0;
}

void festation::DmaControl::write32(uint32_t address, uint32_t value)
{
}
