#include "psx_system.hpp"
#include "virtual_mem_allocator_utils.hpp"
#include "memory_map_masks.hpp"
#include "utils/logger.hpp"

#include <stdlib.h>
#include <assert.h>

festation::PSXSystem::PSXSystem()
    : cpu(this), mainRAM(MAIN_RAM_SIZE), bios(KernelBIOS(cpu))
{
}

festation::PSXSystem::~PSXSystem()
{
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
        return mainRAM[masked_address & MAIN_RAM_SIZE_MASK];
    }
    else if (masked_address >= EXPANSION_REGION1_START && masked_address <= EXPANSION_REGION1_END)
    {
        LOG_WARN("Not implemented read on expansion region 1!");
        assert(false);
    }
    else if (masked_address >= IO_PORTS_START && masked_address <= IO_PORTS_END)
    {
        LOG_DEBUG("Read from I/O port address 0x{:08X}", address);
        //assert(false && "Not implemented read on I/O Ports!");
        return 0xFF;
    }
    else if (masked_address >= EXPANSION_REGION2_START && masked_address <= EXPANSION_REGION2_END)
    {
        LOG_WARN("Not implemented read on expansion region 2!");
        assert(false);
    }
    else if (masked_address >= EXPANSION_REGION3_START && masked_address <= EXPANSION_REGION3_END)
    {
        LOG_WARN("Not implemented read on expansion region 3!");
        assert(false);
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        return bios.read8(masked_address & BIOS_ROM_SIZE_MASK);
    } 
    else if ((address & 0xFFFE0000) == 0xFFFE0000)
    {
        LOG_WARN("Not implemented read on internal CPU control registers!");
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
        mainRAM[masked_address & MAIN_RAM_SIZE_MASK] = value;
    }
    else if (masked_address >= EXPANSION_REGION1_START && masked_address <= EXPANSION_REGION1_END)
    {
        LOG_WARN("Not implemented write on expansion region 1!");
        assert(false);
    }
    else if (masked_address >= IO_PORTS_START && masked_address <= IO_PORTS_END)
    {
        LOG_DEBUG("Write to I/O port address 0x{:08X}", address);
        //assert(false && "Not implemented write on I/O Ports!");
    }
    else if (masked_address >= EXPANSION_REGION2_START && masked_address <= EXPANSION_REGION2_END)
    {
        LOG_WARN("Not implemented write on expansion region 2!");
        assert(false);
    }
    else if (masked_address >= EXPANSION_REGION3_START && masked_address <= EXPANSION_REGION3_END)
    {
        LOG_WARN("Not implemented write on expansion region 3!");
        assert(false);
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        bios.write8(masked_address & BIOS_ROM_SIZE_MASK, value);
    }
    else if ((address & 0xFFFE0000) == 0xFFFE0000)
    {
        LOG_WARN("Not implemented write to internal CPU control registers!");
        assert(false);
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
