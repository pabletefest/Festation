#include "psx_system.hpp"
#include "virtual_mem_allocator_utils.hpp"
#include "memory_map_masks.hpp"

#include <stdlib.h>
#include <assert.h>

festation::PSXSystem::PSXSystem()
    : cpu(this), mainRAM(allocVirtMemForMainRAM()), bios(KernelBIOS())
{
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

// IMPLEMENT READ16 AND READ32 AS MULTIPLE READ8 SIMPLIFIES IMPLEMENTATION
// IF PERFORMANCE IS REDUCED DUE TO OVERHEAD, TRY IMPLEMENT THEM ON THEIR OWN

uint8_t festation::PSXSystem::read8(uint32_t address)
{   
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        return mainRAM[address & MAIN_RAM_SIZE];
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        return bios.read8(address & BIOS_ROM_SIZE);
    }else {
        if ((masked_address & 0x1FF00000) == 0x1F800000)
            assert(false);
    }

    return 0;
}

uint16_t festation::PSXSystem::read16(uint32_t address)
{
    // uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    // if (masked_address <= MAIN_RAM_END)
    // {
    //     return *(uint16_t*)&mainRAM[address & MAIN_RAM_SIZE];
    // }

    // return 0;

    return (read8(address + 1) << 8) | read8(address);
}

uint32_t festation::PSXSystem::read32(uint32_t address)
{
    // uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    // if (masked_address <= MAIN_RAM_END)
    // {
    //     return *(uint32_t*)&mainRAM[address & MAIN_RAM_SIZE];
    // }

    // return 0;

    return (read16(address + 2) << 16) | read16(address);
}

void festation::PSXSystem::write8(uint32_t address, uint8_t value)
{
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        mainRAM[address & MAIN_RAM_SIZE] = value;
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        bios.write8(address & BIOS_ROM_SIZE, value);
    }
}

void festation::PSXSystem::write16(uint32_t address, uint16_t value)
{
//     uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

//     if (masked_address <= MAIN_RAM_END)
//     {
//         *(uint16_t*)&mainRAM[address & MAIN_RAM_SIZE] = value;
//     }

    write8(address, value & 0xFF);
    write8(address + 1, (value >> 8) & 0xFF);
}

void festation::PSXSystem::write32(uint32_t address, uint32_t value)
{
    // uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    // if (masked_address <= MAIN_RAM_END)
    // {
    //     *(uint32_t*)&mainRAM[address & MAIN_RAM_SIZE] = value;
    // }

    write16(address, value & 0xFFFF);
    write16(address + 2, (value >> 16) & 0xFFFF);
}

void festation::PSXSystem::runWholeFrame()
{
    while(true)
    {
        cpu.executeInstruction();
        bios.checkKernerlTTYOutput();
    }
}
