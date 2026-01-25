#include "psx_system.hpp"
#include "virtual_mem_allocator_utils.hpp"
#include "memory_map_masks.hpp"
#include "utils/logger.hpp"
#include "utils/file_reader.hpp"

#include <stdlib.h>
#include <assert.h>

static constexpr const uint32_t CYCLES_FER_FRAME_NTSC = 897619;

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
        LOG_WARN("Not implemented read8 at 0x{:08X} on expansion region 1!", address);
        return 0xFF; // Stub
    }
    else if (masked_address >= IO_PORTS_START && masked_address <= IO_PORTS_END)
    {
        LOG_DEBUG("Read8 from I/O port address 0x{:08X}", address);
        //assert(false && "Not implemented read on I/O Ports!");
        return 0xFF;
    }
    else if (masked_address >= EXPANSION_REGION2_START && masked_address <= EXPANSION_REGION2_END)
    {
        LOG_WARN("Not implemented read8 at 0x{:08X} on expansion region 2!", address);
        return 0xFF;
    }
    else if (masked_address >= EXPANSION_REGION3_START && masked_address <= EXPANSION_REGION3_END)
    {
        LOG_WARN("Not implemented read8 at 0x{:08X} on expansion region 3!", address);
        return 0xFF;
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        return bios.read8(masked_address & BIOS_ROM_SIZE_MASK);
    } 
    else if ((address & 0xFFFE0000) == 0xFFFE0000)
    {
        LOG_WARN("Not implemented read8 on internal CPU control registers at address 0x{:08X}!", address);
    }

    return 0;
}

uint16_t festation::PSXSystem::read16(uint32_t address)
{
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        return *(uint16_t*)&mainRAM[masked_address & MAIN_RAM_SIZE_MASK];
    }
    else if (masked_address >= EXPANSION_REGION1_START && masked_address <= EXPANSION_REGION1_END)
    {
        LOG_WARN("Not implemented read16 at 0x{:08X} on expansion region 1!", address);
        return 0xFFFF; // Stub
    }
    else if (masked_address >= IO_PORTS_START && masked_address <= IO_PORTS_END)
    {
        LOG_DEBUG("Read16 from I/O port address 0x{:08X}", address);
        //assert(false && "Not implemented read on I/O Ports!");
        return 0xFFFF;
    }
    else if (masked_address >= EXPANSION_REGION2_START && masked_address <= EXPANSION_REGION2_END)
    {
        LOG_WARN("Not implemented read16 at 0x{:08X} on expansion region 2!", address);
        return 0xFFFF;
    }
    else if (masked_address >= EXPANSION_REGION3_START && masked_address <= EXPANSION_REGION3_END)
    {
        LOG_WARN("Not implemented read16 at 0x{:08X} on expansion region 3!", address);
        return 0xFFFF;
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        return bios.read16(masked_address & BIOS_ROM_SIZE_MASK);
    }
    else if ((address & 0xFFFE0000) == 0xFFFE0000)
    {
        LOG_WARN("Not implemented read16 on internal CPU control registers at address 0x{:08X}!", address);
    }

    return 0;
}

uint32_t festation::PSXSystem::read32(uint32_t address)
{
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        return *(uint32_t*)&mainRAM[masked_address & MAIN_RAM_SIZE_MASK];
    }
    else if (masked_address >= EXPANSION_REGION1_START && masked_address <= EXPANSION_REGION1_END)
    {
        LOG_WARN("Not implemented read32 at 0x{:08X} on expansion region 1!", address);
        return 0xFFFFFFFF; // Stub
    }
    else if (masked_address >= IO_PORTS_START && masked_address <= IO_PORTS_END)
    {
        switch(masked_address) {
        case 0x1F801810:
        case 0x1F801814:
        {
            uint32_t readValue = gpu.read32(masked_address);
            LOG_DEBUG("Reading {:08X}h from GPU IO port 0x{:08X}", readValue, address);
            return readValue;
        }
        default:
            LOG_DEBUG("Read32 from I/O port address 0x{:08X}", address);
            break;
        }

        //assert(false && "Not implemented read on I/O Ports!");
        return 0xFFFFFFFF;
    }
    else if (masked_address >= EXPANSION_REGION2_START && masked_address <= EXPANSION_REGION2_END)
    {
        LOG_WARN("Not implemented read32 at 0x{:08X} on expansion region 2!", address);
        return 0xFFFFFFFF;
    }
    else if (masked_address >= EXPANSION_REGION3_START && masked_address <= EXPANSION_REGION3_END)
    {
        LOG_WARN("Not implemented read32 at 0x{:08X} on expansion region 3!", address);
        return 0xFFFFFFFF;
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        return bios.read32(masked_address & BIOS_ROM_SIZE_MASK);
    }
    else if ((address & 0xFFFE0000) == 0xFFFE0000)
    {
        LOG_WARN("Not implemented read32 on internal CPU control registers at address 0x{:08X}!", address);
    }

    return 0;
}

void festation::PSXSystem::write8(uint32_t address, uint8_t value)
{
    if (cpu.isCacheIsolated())
        return;

    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        mainRAM[masked_address & MAIN_RAM_SIZE_MASK] = value;
    }
    else if (masked_address >= EXPANSION_REGION1_START && masked_address <= EXPANSION_REGION1_END)
    {
        LOG_WARN("Not implemented write8 to 0x{:08X} on expansion region 1!", address);
    }
    else if (masked_address >= IO_PORTS_START && masked_address <= IO_PORTS_END)
    {
        LOG_DEBUG("Write8 ({:02X}h) to I/O port address 0x{:08X}", value, address);
        //assert(false && "Not implemented write on I/O Ports!");
    }
    else if (masked_address >= EXPANSION_REGION2_START && masked_address <= EXPANSION_REGION2_END)
    {
        LOG_WARN("Not implemented write8 to 0x{:08X} on expansion region 2!", address);
    }
    else if (masked_address >= EXPANSION_REGION3_START && masked_address <= EXPANSION_REGION3_END)
    {
        LOG_WARN("Not implemented write8 to 0x{:08X} on expansion region 3!", address);
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        bios.write8(masked_address & BIOS_ROM_SIZE_MASK, value);
    }
    else if ((address & 0xFFFE0000) == 0xFFFE0000)
    {
        LOG_WARN("Not implemented write8 to internal CPU control registers 0x{:08X}!", address);
    }
}

void festation::PSXSystem::write16(uint32_t address, uint16_t value)
{
    if (cpu.isCacheIsolated())
        return;

    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        *(uint16_t*)&mainRAM[masked_address & MAIN_RAM_SIZE_MASK] = value;
    }
    else if (masked_address >= EXPANSION_REGION1_START && masked_address <= EXPANSION_REGION1_END)
    {
        LOG_WARN("Not implemented write16 to 0x{:08X} on expansion region 1!", address);
    }
    else if (masked_address >= IO_PORTS_START && masked_address <= IO_PORTS_END)
    {
        LOG_DEBUG("Write16 ({:04X}h) to I/O port address 0x{:08X}", value, address);
        //assert(false && "Not implemented write on I/O Ports!");
    }
    else if (masked_address >= EXPANSION_REGION2_START && masked_address <= EXPANSION_REGION2_END)
    {
        LOG_WARN("Not implemented write16 to 0x{:08X} on expansion region 2!", address);
    }
    else if (masked_address >= EXPANSION_REGION3_START && masked_address <= EXPANSION_REGION3_END)
    {
        LOG_WARN("Not implemented write16 to 0x{:08X} on expansion region 3!", address);
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        bios.write16(masked_address & BIOS_ROM_SIZE_MASK, value);
    }
    else if ((address & 0xFFFE0000) == 0xFFFE0000)
    {
        LOG_WARN("Not implemented write16 to internal CPU control registers 0x{:08X}!", address);
    }
}

void festation::PSXSystem::write32(uint32_t address, uint32_t value)
{
    if (cpu.isCacheIsolated())
        return;

    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        *(uint32_t*)&mainRAM[masked_address & MAIN_RAM_SIZE_MASK] = value;
    }
    else if (masked_address >= EXPANSION_REGION1_START && masked_address <= EXPANSION_REGION1_END)
    {
        LOG_WARN("Not implemented write32 to 0x{:08X} on expansion region 1!", address);
    }
    else if (masked_address >= IO_PORTS_START && masked_address <= IO_PORTS_END)
    {
        switch(masked_address) {
        case 0x1F801810:
        case 0x1F801814:
            LOG_DEBUG("Writting {:08X}h to GPU IO port 0x{:08X}", value, address);
            gpu.write32(masked_address, value);
            break;
        default:
            LOG_DEBUG("Write32 ({:08X}h) to I/O port address 0x{:08X}", value, address);
            break;
        }

        //assert(false && "Not implemented write on I/O Ports!");
    }
    else if (masked_address >= EXPANSION_REGION2_START && masked_address <= EXPANSION_REGION2_END)
    {
        LOG_WARN("Not implemented write32 to 0x{:08X} on expansion region 2!", address);
    }
    else if (masked_address >= EXPANSION_REGION3_START && masked_address <= EXPANSION_REGION3_END)
    {
        LOG_WARN("Not implemented write32 to 0x{:08X} on expansion region 3!", address);
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        bios.write32(masked_address & BIOS_ROM_SIZE_MASK, value);
    }
    else if ((address & 0xFFFE0000) == 0xFFFE0000)
    {
        LOG_WARN("Not implemented write32 to internal CPU control registers 0x{:08X}!", address);
    }
}

void festation::PSXSystem::runWholeFrame()
{
    int32_t totalFrameCycles = CYCLES_FER_FRAME_NTSC;

    while (totalFrameCycles > 0) {
        uint8_t cycles = cpu.executeInstruction();
        bios.checkKernerlTTYOutput();
        totalElapsedCycles += cycles;
        totalFrameCycles -= cycles;
    }
}

void festation::PSXSystem::sideloadExeFile(const std::filesystem::path& path)
{
    uint32_t& pcRef = cpu.getCPURegs().pc;

    while (pcRef != 0x80030000)
    {
        uint8_t cycles = cpu.executeInstruction();
        bios.checkKernerlTTYOutput();
        totalElapsedCycles += cycles;
    }

    LOG_INFO("READY TO SIDELOAD EXEs!");

    std::vector<uint8_t> exe = festation::readFile<uint8_t>(path);
    
    constexpr size_t HEADER_SIZE = 2048;

    uint32_t initialPC = *reinterpret_cast<uint32_t*>(&exe[0x10]);
    uint32_t initialR28 = *reinterpret_cast<uint32_t*>(&exe[0x14]);
    uint32_t startExeRamAddress = *reinterpret_cast<uint32_t*>(&exe[0x18]) & MAIN_RAM_SIZE_MASK;
    uint32_t exeSize = *reinterpret_cast<uint32_t*>(&exe[0x1C]); // 2KB multiples
    uint32_t initialR29_R30 = *reinterpret_cast<uint32_t*>(&exe[0x30]);

    cpu.getCPURegs().gpr_regs[28] = initialR28;

    if (initialR29_R30 != 0) {
        cpu.getCPURegs().gpr_regs[29] = initialR29_R30;
        cpu.getCPURegs().gpr_regs[30] = initialR29_R30;
    }

    std::memcpy(mainRAM.data() + startExeRamAddress, 
        exe.data() + HEADER_SIZE, exeSize);

    cpu.getCPURegs().pc = initialPC;
}