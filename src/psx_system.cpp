#include "psx_system.hpp"
#include "virtual_mem_allocator_utils.hpp"
#include "memory_map_masks.hpp"

#include <stdlib.h>

festation::PSXSystem::PSXSystem()
    : cpu(this)
{
    mainRAM = allocVirtMemForMainRAM();

    if (!mainRAM)
        mainRAM = (uint8_t*)malloc(2 * 1024 * 1024);
}

festation::PSXSystem::~PSXSystem()
{
    if (deallocVirtMemForMainRAM(mainRAM) == -1)
    {
        free(mainRAM);
    }

    mainRAM = nullptr;
}

void festation::PSXSystem::reset()
{
    cpu.reset();
}

uint8_t festation::PSXSystem::read8(uint32_t address)
{   
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        return mainRAM[address & MAIN_RAM_SIZE];
    }

    return 0;
}

uint16_t festation::PSXSystem::read16(uint32_t address)
{
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        return *(uint16_t*)&mainRAM[address & MAIN_RAM_SIZE];
    }

    return 0;
}

uint32_t festation::PSXSystem::read32(uint32_t address)
{
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        return *(uint32_t*)&mainRAM[address & MAIN_RAM_SIZE];
    }

    return 0;
}

void festation::PSXSystem::write8(uint32_t address, uint8_t value)
{
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        mainRAM[address & MAIN_RAM_SIZE] = value;
    }
}

void festation::PSXSystem::write16(uint32_t address, uint16_t value)
{
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        *(uint16_t*)&mainRAM[address & MAIN_RAM_SIZE] = value;
    }
}

void festation::PSXSystem::write32(uint32_t address, uint32_t value)
{
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        *(uint32_t*)&mainRAM[address & MAIN_RAM_SIZE] = value;
    }
}

void festation::PSXSystem::runWholeFrame()
{

}
