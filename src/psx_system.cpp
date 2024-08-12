#include "psx_system.h"
#include "virtual_mem_allocator_utils.h"

#include <stdlib.h>

festation::PSXSystem::PSXSystem()
    : cpu(this)
{
    mainRAM = allocateVirtMemForMainRAM();

    if (!mainRAM)
        mainRAM = (uint8_t*)malloc(2 * 1024 * 1024);
}

festation::PSXSystem::~PSXSystem()
{
    if (deallocateVirtMemForMainRAM(mainRAM) == -1)
    {
        free(mainRAM);
    }

    mainRAM = nullptr;
}

uint8_t festation::PSXSystem::read8(uint32_t address)
{
    return mainRAM[address];
}

uint16_t festation::PSXSystem::read16(uint32_t address)
{
    return *(uint16_t*)&mainRAM[address];
}

uint32_t festation::PSXSystem::read32(uint32_t address)
{
    return *(uint32_t*)&mainRAM[address];
}

void festation::PSXSystem::write8(uint32_t address, uint8_t value)
{
    mainRAM[address] = value;
}

void festation::PSXSystem::write16(uint32_t address, uint16_t value)
{
    *(uint16_t*)&mainRAM[address] = value;
}

void festation::PSXSystem::write32(uint32_t address, uint32_t value)
{
    *(uint32_t*)&mainRAM[address] = value;
}

void festation::PSXSystem::runWholeFrame()
{

}
