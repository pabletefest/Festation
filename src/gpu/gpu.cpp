#include "gpu.hpp"

festation::PsxGpu::PsxGpu()
    : GPUREAD({}), GPUSTAT({})
{
}

festation::PsxGpu::~PsxGpu()
{
}

uint32_t festation::PsxGpu::read32(uint32_t address)
{
    switch(address) {
    case 0x1F801810:
        return GPUREAD.raw;
    case 0x1F801814:
        return GPUSTAT.raw;
    default:
        return 0xFFFFFFFF;
    }
}

void festation::PsxGpu::write32(uint32_t address, uint32_t value)
{
    switch(address) {
    case 0x1F801810:
        GPUREAD.raw = value;
    case 0x1F801814:
        GPUSTAT.raw = value;
    default:
        break;
    }
}
