#include "psx_system.hpp"
#include "cdrom/cdrom.hpp"
#include "interrupts/interrupts.hpp"
#include "memory/memory_map_masks.hpp"
#include "utils/logger.hpp"
#include "utils/file_reader.hpp"

#include <stdlib.h>
#include <assert.h>
#include <cstring>
#include <utility>

static constexpr const uint32_t CYCLES_FER_FRAME_NTSC = 565'045;
static bool canSend = false;
static uint8_t currentByte = 0;

festation::PSXSystem::PSXSystem()
    : m_cpu(this, m_interruptsHandler), m_mainRAM(MAIN_RAM_SIZE), m_bios(KernelBIOS(m_cpu)),
        m_cdrom(m_interruptsHandler, m_scheduler) , m_dma(*this), 
            m_timers({{m_interruptsHandler, m_scheduler}, {m_interruptsHandler, m_scheduler}, {m_interruptsHandler, m_scheduler}})
{
    m_scheduler.scheduleEvent({ EventType::VBlank, CYCLES_FER_FRAME_NTSC, 
        [this]() {
            onFrameEnded();
        } });
}

festation::PSXSystem::~PSXSystem()
{
}

auto festation::PSXSystem::reset() -> void
{
    m_cpu.reset();
    m_dma.reset();
    m_mainRAM.clear();
    m_scheduler = Scheduler();
    m_totalElapsedCycles = 0;
    /** @todo Reset the rest of the components.  */
    // m_interruptsHandler.reset();
}

// IMPLEMENT READ16 AND READ32 AS MULTIPLE READ8 SIMPLIFIES IMPLEMENTATION
// IF PERFORMANCE IS REDUCED DUE TO OVERHEAD, TRY IMPLEMENT THEM ON THEIR OWN

auto festation::PSXSystem::read8(uint32_t address) -> uint8_t
{   
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        return m_mainRAM[masked_address & MAIN_RAM_SIZE_MASK];
    }
    else if (masked_address >= EXPANSION_REGION1_START && masked_address <= EXPANSION_REGION1_END)
    {
        // LOG_WARN("Not implemented read8 at 0x{:08X} on expansion region 1!", masked_address);
        return 0; // Stub
    }
    else if (masked_address >= IO_PORTS_START && masked_address <= IO_PORTS_END)
    {
        uint8_t readValue = 0;

        switch(masked_address)
        {
        case 0x1F801070:
            LOG_DEBUG("Read8 from I_STAT INT port 0x{:08X}", masked_address);
            break;
        case 0x1F801074:
            LOG_DEBUG("Read8 from I_MASK INT port 0x{:08X}", masked_address);
            break;
        case 0x1F801040:
            LOG_DEBUG("Read8 from Joypad/memory Card port DATA 0x{:08X}", masked_address);
            {
                static uint8_t stubStartResponse[5] = { 0xFF, 0x41, 0x5A, 0xFF, 0xBF };

                if (!canSend)
                    return 0xFF;

                uint8_t byte = stubStartResponse[currentByte++];

                if (currentByte == 5)
                {
                    canSend = false;
                    currentByte = 0;
                }

                return byte;
            }
            break;
        case 0x1F801044:
            LOG_DEBUG("Read8 from Joypad/memory Card port STAT 0x{:08X}", masked_address);
            break;
        case 0x1F801048:
            LOG_DEBUG("Read8 from Joypad/memory Card port MODE 0x{:08X}", masked_address);
            break;
        case 0x1F80104A:
            LOG_DEBUG("Read8 from Joypad/memory Card port CTRL 0x{:08X}", masked_address);
            break;
        case 0x1F80104E:
            LOG_DEBUG("Read8 from Joypad/memory Card port BAUD 0x{:08X}", masked_address);
            break;
        default:
            if (masked_address >= 0x1F801100 && masked_address <= 0x1F80112F)
            {
                size_t timerId = (masked_address >> 4) & 3;
                readValue = m_timers[timerId].read8(masked_address);
                LOG_DEBUG("Read8 ({:02X}h) from Timer port address 0x{:08X}", readValue,  masked_address);
            }
            else if (masked_address >= 0x1F801800 && masked_address <= 0x1F801803)
            {
                readValue = m_cdrom.read8(address);
                LOG_DEBUG("Read8 ({:02X}h) from CDROM port address 0x{:08X}",readValue,  masked_address);
            }
            else
            {
                LOG_DEBUG("Read8 from I/O port address 0x{:08X}", masked_address);
            }

            break;
        }

        return readValue;
    }
    else if (masked_address >= EXPANSION_REGION2_START && masked_address <= EXPANSION_REGION2_END)
    {
        // LOG_WARN("Not implemented read8 at 0x{:08X} on expansion region 2!", masked_address);
        return 0;
    }
    else if (masked_address >= EXPANSION_REGION3_START && masked_address <= EXPANSION_REGION3_END)
    {
        // LOG_WARN("Not implemented read8 at 0x{:08X} on expansion region 3!", masked_address);
        return 0;
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        return m_bios.read8(masked_address & BIOS_ROM_SIZE_MASK);
    } 
    else if ((address & 0xFFFE0000) == 0xFFFE0000)
    {
        LOG_WARN("Not implemented read8 on internal CPU control registers at address 0x{:08X}!", address);
    }

    return 0;
}

auto festation::PSXSystem::read16(uint32_t address) -> uint16_t
{
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        return *(uint16_t*)&m_mainRAM[masked_address & MAIN_RAM_SIZE_MASK];
    }
    else if (masked_address >= EXPANSION_REGION1_START && masked_address <= EXPANSION_REGION1_END)
    {
        // LOG_WARN("Not implemented read16 at 0x{:08X} on expansion region 1!", masked_address);
        return 0; // Stub
    }
    else if (masked_address >= IO_PORTS_START && masked_address <= IO_PORTS_END)
    {
        uint16_t readValue = 0;

        switch(masked_address)
        {
        case 0x1F801070:
            readValue = m_interruptsHandler.read16(masked_address);
            LOG_DEBUG("Read16 ({:04X}h) from I_STAT INT port 0x{:08X}", readValue, masked_address);
            break;
        case 0x1F801074:
            readValue = m_interruptsHandler.read16(masked_address);
            LOG_DEBUG("Read16 ({:04X}h) from I_MASK INT port 0x{:08X}", readValue, masked_address);
            break;
        case 0x1F801040:
            LOG_DEBUG("Read16 from Joypad/Memory Card DATA port 0x{:08X}", masked_address);
            readValue = 0xFFFF;
            break;
        case 0x1F801044:
            LOG_DEBUG("Read16 from Joypad/Memory Card STAT port 0x{:08X}", masked_address);
            readValue = 0xFFFF;
            break;
        case 0x1F801048:
            LOG_DEBUG("Read16 from Joypad/Memory Card MODE port 0x{:08X}", masked_address);
            readValue = 0xFFFF;
            break;
        case 0x1F80104A:
            LOG_DEBUG("Read16 from Joypad/Memory Card CTRL port 0x{:08X}", masked_address);
            readValue = 0x3FAF;
            break;
        case 0x1F80104E:
            LOG_DEBUG("Read16 from Joypad/Memory Card BAUD port 0x{:08X}", masked_address);
            readValue = 0xFFFF;
            break;
        default:
            if (masked_address >= 0x1F801100 && masked_address <= 0x1F80112F)
            {
                size_t timerId = (masked_address >> 4) & 3;
                readValue = m_timers[timerId].read16(masked_address);
                LOG_DEBUG("Read16 ({:04X}h) from Timer port address 0x{:08X}", readValue, masked_address);
            }
            else if (masked_address >= 0x1F801800 && masked_address <= 0x1F801803)
            {
                LOG_DEBUG("Read16 from CDROM port address 0x{:08X}", masked_address);
                readValue = m_cdrom.read16(masked_address);
            }
            else
            {
                // LOG_DEBUG("Read16 from I/O port address 0x{:08X}", masked_address);
            }

            break;
        }

        return readValue;
    }
    else if (masked_address >= EXPANSION_REGION2_START && masked_address <= EXPANSION_REGION2_END)
    {
        // LOG_WARN("Not implemented read16 at 0x{:08X} on expansion region 2!", masked_address);
        return 0;
    }
    else if (masked_address >= EXPANSION_REGION3_START && masked_address <= EXPANSION_REGION3_END)
    {
        // LOG_WARN("Not implemented read16 at 0x{:08X} on expansion region 3!", masked_address);
        return 0;
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        return m_bios.read16(masked_address & BIOS_ROM_SIZE_MASK);
    }
    else if ((address & 0xFFFE0000) == 0xFFFE0000)
    {
        LOG_WARN("Not implemented read16 on internal CPU control registers at address 0x{:08X}!", address);
    }

    return 0;
}

auto festation::PSXSystem::read32(uint32_t address) -> uint32_t
{
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        return *(uint32_t*)&m_mainRAM[masked_address & MAIN_RAM_SIZE_MASK];
    }
    else if (masked_address >= EXPANSION_REGION1_START && masked_address <= EXPANSION_REGION1_END)
    {
        // LOG_WARN("Not implemented read32 at 0x{:08X} on expansion region 1!", masked_address);
        return 0; // Stub
    }
    else if (masked_address >= IO_PORTS_START && masked_address <= IO_PORTS_END)
    {
        uint32_t readValue = 0;

        switch(masked_address)
        {
        case 0x1F801810:
        case 0x1F801814:
        {
            readValue = m_gpu.read32(masked_address);
            // LOG_DEBUG("Reading {:08X}h from GPU IO port 0x{:08X}", readValue, masked_address);
            break;
        }
        case 0x1F801070:
            readValue = m_interruptsHandler.read32(address);
            LOG_DEBUG("Read32 ({:08X}h) from I_STAT INT port 0x{:08X}", readValue, masked_address);
            break;
        case 0x1F801074:
            readValue = m_interruptsHandler.read32(address);
            LOG_DEBUG("Read32 ({:08X}h) from I_MASK INT port 0x{:08X}", readValue, masked_address);
            break;
        case 0x1F801040:
            LOG_DEBUG("Read32 from Joypad/Memory Card DATA port 0x{:08X}", masked_address);
            readValue = 0xFFFFFFFF;
            break;
        case 0x1F801044:
            LOG_DEBUG("Read32 from Joypad/Memory Card STAT port 0x{:08X}", masked_address);
            readValue = 0xFFFFFFFF;
            break;
        case 0x1F80104E:
            LOG_DEBUG("Read32 from Joypad/Memory Card BAUD port 0x{:08X}", masked_address);
            readValue = 0xFFFFFFFF;
            break;
        default:
            if (masked_address >= 0x1F801080 && masked_address <= 0x1F8010FF)
            {
                readValue = m_dma.read32(masked_address);
                // LOG_DEBUG("Reading {:08X}h from DMA IO port 0x{:08X}", readValue, masked_address);
            }
            else if (masked_address >= 0x1F801100 && masked_address <= 0x1F80112F)
            {
                size_t timerId = (masked_address >> 4) & 3;
                readValue = m_timers[timerId].read32(masked_address);
                LOG_DEBUG("Read32 ({:08X}h) from Timer port address 0x{:08X}", readValue, masked_address);
            }
            else if (masked_address >= 0x1F801800 && masked_address <= 0x1F801803)
            {
                LOG_DEBUG("Read32 from CDROM port address 0x{:08X}", masked_address);
                readValue = m_cdrom.read32(masked_address);
            }
            else
            {
                // LOG_DEBUG("Read32 from I/O port address 0x{:08X}", masked_address);
            }

            break;
        }

        return readValue;
    }
    else if (masked_address >= EXPANSION_REGION2_START && masked_address <= EXPANSION_REGION2_END)
    {
        // LOG_WARN("Not implemented read32 at 0x{:08X} on expansion region 2!", masked_address);
        return 0;
    }
    else if (masked_address >= EXPANSION_REGION3_START && masked_address <= EXPANSION_REGION3_END)
    {
        // LOG_WARN("Not implemented read32 at 0x{:08X} on expansion region 3!", masked_address);
        return 0;
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        return m_bios.read32(masked_address & BIOS_ROM_SIZE_MASK);
    }
    else if ((address & 0xFFFE0000) == 0xFFFE0000)
    {
        LOG_WARN("Not implemented read32 on internal CPU control registers at address 0x{:08X}!", address);
    }

    return 0;
}

auto festation::PSXSystem::write8(uint32_t address, uint8_t value) -> void
{
    if (m_cpu.isCacheIsolated())
        return;

    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        m_mainRAM[masked_address & MAIN_RAM_SIZE_MASK] = value;
    }
    else if (masked_address >= EXPANSION_REGION1_START && masked_address <= EXPANSION_REGION1_END)
    {
        // LOG_WARN("Not implemented write8 to 0x{:08X} on expansion region 1!", masked_address);
    }
    else if (masked_address >= IO_PORTS_START && masked_address <= IO_PORTS_END)
    {
        switch(masked_address)
        {
        case 0x1F801070:
            LOG_DEBUG("Write8 ({:02X}h) to I_STAT INT port 0x{:08X}", masked_address);
            break;
        case 0x1F801074:
            LOG_DEBUG("Write8 ({:02X}h) to I_MASK INT port 0x{:08X}", masked_address);
            break;
        case 0x1F801040:
            {
                if (value == 0x01) {
                    currentByte = 0;
                    canSend = true;
                } 
            }
            LOG_DEBUG("Write8 ({:02X}h) to Joypad/Memory Card DATA port 0x{:08X}", value, masked_address);
            break;
        case 0x1F801048:
            LOG_DEBUG("Write8 ({:02X}h) to Joypad/Memory Card MODE port 0x{:08X}", value, masked_address);
            break;
        case 0x1F80104A:
            LOG_DEBUG("Write8 ({:02X}h) to Joypad/Memory Card CTRL port 0x{:08X}", value, masked_address);
            break;
        case 0x1F80104E:
            LOG_DEBUG("Write8 ({:02X}h) to Joypad/Memory Card BAUD port 0x{:08X}", value, masked_address);
            break;
        default:
            if (masked_address >= 0x1F801100 && masked_address <= 0x1F80112F)
            {
                LOG_DEBUG("Write8 ({:02X}h) to Timer port address 0x{:08X}", value, masked_address);
                size_t timerId = (masked_address >> 4) & 3;
                m_timers[timerId].write8(masked_address, value);
            }
            else if (masked_address >= 0x1F801800 && masked_address <= 0x1F801803)
            {
                LOG_DEBUG("Write8 ({:02X}h) to CDROM port address 0x{:08X}", value, masked_address);
                m_cdrom.write8(address, value);
            }
            else
            {
                LOG_DEBUG("Write8 ({:02X}h) to I/O port address 0x{:08X}", value, masked_address);
            }

            break;
        }
    }
    else if (masked_address >= EXPANSION_REGION2_START && masked_address <= EXPANSION_REGION2_END)
    {
        // LOG_WARN("Not implemented write8 to 0x{:08X} on expansion region 2!", masked_address);
    }
    else if (masked_address >= EXPANSION_REGION3_START && masked_address <= EXPANSION_REGION3_END)
    {
        // LOG_WARN("Not implemented write8 to 0x{:08X} on expansion region 3!", masked_address);
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        m_bios.write8(masked_address & BIOS_ROM_SIZE_MASK, value);
    }
    else if ((address & 0xFFFE0000) == 0xFFFE0000)
    {
        LOG_WARN("Not implemented write8 to internal CPU control registers 0x{:08X}!", address);
    }
}

auto festation::PSXSystem::write16(uint32_t address, uint16_t value) -> void
{
    if (m_cpu.isCacheIsolated())
        return;

    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        *(uint16_t*)&m_mainRAM[masked_address & MAIN_RAM_SIZE_MASK] = value;
    }
    else if (masked_address >= EXPANSION_REGION1_START && masked_address <= EXPANSION_REGION1_END)
    {
        // LOG_WARN("Not implemented write16 to 0x{:08X} on expansion region 1!", masked_address);
    }
    else if (masked_address >= IO_PORTS_START && masked_address <= IO_PORTS_END)
    {
        switch(masked_address)
        {
        case 0x1F801070:
            LOG_DEBUG("Write16 ({:04X}h) to I_STAT INT port 0x{:08X}", value, masked_address);
            m_interruptsHandler.write16(masked_address, value);
            break;
        case 0x1F801074:
            LOG_DEBUG("Write16 ({:04X}h) to I_MASK INT port 0x{:08X}", value, masked_address);
            m_interruptsHandler.write16(masked_address, value);
            break;
        case 0x1F801040:
            LOG_DEBUG("Write16 ({:04X}h) to Joypad/Memory Card DATA port 0x{:08X}", value, masked_address);
            break;
        case 0x1F801048:
            LOG_DEBUG("Write16 ({:04X}h) to Joypad/Memory Card MODE port 0x{:08X}", value, masked_address);
            break;
        case 0x1F80104A:
            LOG_DEBUG("Write16 ({:04X}h) to Joypad/Memory Card CTRL port 0x{:08X}", value, masked_address);
            break;
        case 0x1F80104E:
            LOG_DEBUG("Write16 ({:04X}h) to Joypad/Memory Card BAUD port 0x{:08X}", value, masked_address);
            break;
        default:
            if (masked_address >= 0x1F801100 && masked_address <= 0x1F80112F)
            {
                LOG_DEBUG("Write16 ({:04X}h) to Timer port address 0x{:08X}", value, masked_address);
                size_t timerId = (masked_address >> 4) & 3;
                m_timers[timerId].write16(masked_address, value);
            }
            else if (masked_address >= 0x1F801800 && masked_address <= 0x1F801803)
            {
                LOG_DEBUG("Write16 ({:04X}h) to CDROM port address 0x{:08X}", value, masked_address);
                std::unreachable();
            }
            else
            {
                // LOG_DEBUG("Write16 ({:04X}h) to I/O port address 0x{:08X}", value, masked_address);
            }

            break;
        }
    }
    else if (masked_address >= EXPANSION_REGION2_START && masked_address <= EXPANSION_REGION2_END)
    {
        // LOG_WARN("Not implemented write16 to 0x{:08X} on expansion region 2!", masked_address);
    }
    else if (masked_address >= EXPANSION_REGION3_START && masked_address <= EXPANSION_REGION3_END)
    {
        // LOG_WARN("Not implemented write16 to 0x{:08X} on expansion region 3!", masked_address);
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        m_bios.write16(masked_address & BIOS_ROM_SIZE_MASK, value);
    }
    else if ((address & 0xFFFE0000) == 0xFFFE0000)
    {
        LOG_WARN("Not implemented write16 to internal CPU control registers 0x{:08X}!", address);
    }
}

auto festation::PSXSystem::write32(uint32_t address, uint32_t value) -> void
{
    if (m_cpu.isCacheIsolated())
        return;

    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address <= MAIN_RAM_END)
    {
        *(uint32_t*)&m_mainRAM[masked_address & MAIN_RAM_SIZE_MASK] = value;
    }
    else if (masked_address >= EXPANSION_REGION1_START && masked_address <= EXPANSION_REGION1_END)
    {
        // LOG_WARN("Not implemented write32 to 0x{:08X} on expansion region 1!", masked_address);
    }
    else if (masked_address >= IO_PORTS_START && masked_address <= IO_PORTS_END)
    {
        switch(masked_address)
        {
        case 0x1F801810:
        case 0x1F801814:
            // LOG_DEBUG("Writting {:08X}h to GPU IO port 0x{:08X}", value, masked_address);
            m_gpu.write32(masked_address, value);
            break;
        case 0x1F801070:
            LOG_DEBUG("Write32 ({:08X}h) to I_STAT INT port 0x{:08X}", value, masked_address);
            m_interruptsHandler.write32(address, value);
            break;
        case 0x1F801074:
            LOG_DEBUG("Write32 ({:08X}h) to I_MASK INT port 0x{:08X}", value, masked_address);
            m_interruptsHandler.write32(address, value);
            break;
        case 0x1F801040:
            LOG_DEBUG("Write32 ({:08X}h) to Joypad/Memory Card port DATA 0x{:08X}", value, masked_address);
            break;
        case 0x1F80104E:
            LOG_DEBUG("Write32 ({:08X}h) to Joypad/Memory Card port DATA 0x{:08X}", value, masked_address);
            break;
        default:
            if (masked_address >= 0x1F801080 && masked_address <= 0x1F8010FF)
            {
                // LOG_DEBUG("Writting {:08X}h to DMA IO port 0x{:08X}", value, masked_address);
                m_dma.write32(masked_address, value);
            }
            else if (masked_address >= 0x1F801100 && masked_address <= 0x1F80112F)
            {
                LOG_DEBUG("Write32 ({:08X}h) to Timer port address 0x{:08X}", value, masked_address);
                size_t timerId = (masked_address >> 4) & 3;
                m_timers[timerId].write32(masked_address, value);
            }
            else if (masked_address >= 0x1F801800 && masked_address <= 0x1F801803)
            {
                LOG_DEBUG("Write32 ({:08X}h) to CDROM port address 0x{:08X}", value, masked_address);
                std::unreachable();
            }
            else
            {
                LOG_DEBUG("Write32 ({:08X}h) to I/O port address 0x{:08X}", value, masked_address);
            }

            break;
        }
    }
    else if (masked_address >= EXPANSION_REGION2_START && masked_address <= EXPANSION_REGION2_END)
    {
        // LOG_WARN("Not implemented write32 to 0x{:08X} on expansion region 2!", masked_address);
    }
    else if (masked_address >= EXPANSION_REGION3_START && masked_address <= EXPANSION_REGION3_END)
    {
        // LOG_WARN("Not implemented write32 to 0x{:08X} on expansion region 3!", masked_address);
    }
    else if (masked_address >= BIOS_ROM_START && masked_address <= BIOS_ROM_END)
    {
        m_bios.write32(masked_address & BIOS_ROM_SIZE_MASK, value);
    }
    else if ((address & 0xFFFE0000) == 0xFFFE0000)
    {
        LOG_WARN("Not implemented write32 to internal CPU control registers 0x{:08X}!", address);
    }
}

auto festation::PSXSystem::run() -> void
{
    uint8_t cycles = m_cpu.executeInstruction();
    m_bios.checkKernerlTTYOutput();
    m_scheduler.step(cycles);
    m_totalElapsedCycles += cycles;
}

auto festation::PSXSystem::runWholeFrame() -> void
{
    int32_t totalFrameCycles = CYCLES_FER_FRAME_NTSC;

    while (totalFrameCycles > 0) {
        uint8_t cycles = m_cpu.executeInstruction();
        m_bios.checkKernerlTTYOutput();
        m_totalElapsedCycles += cycles;
        totalFrameCycles -= cycles;
    }
    
    m_interruptsHandler.setInterruptSource(festation::InterruptSource::VBlankSrc);
    m_gpu.renderFrame();
}

auto festation::PSXSystem::sideloadExeFile(const std::filesystem::path& path) -> void
{
    uint32_t& pcRef = m_cpu.getCPURegs().pc;

    while (pcRef != 0x80030000)
    {
        uint8_t cycles = m_cpu.executeInstruction();
        m_bios.checkKernerlTTYOutput();
        m_totalElapsedCycles += cycles;
    }

    LOG_INFO("READY TO SIDELOAD EXEs!");

    std::vector<uint8_t> exe = festation::readFile<uint8_t>(path);
    
    constexpr size_t HEADER_SIZE = 2048;

    uint32_t initialPC = *reinterpret_cast<uint32_t*>(&exe[0x10]);
    uint32_t initialR28 = *reinterpret_cast<uint32_t*>(&exe[0x14]);
    uint32_t startExeRamAddress = *reinterpret_cast<uint32_t*>(&exe[0x18]) & MAIN_RAM_SIZE_MASK;
    uint32_t exeSize = *reinterpret_cast<uint32_t*>(&exe[0x1C]); // 2KB multiples
    uint32_t initialR29_R30 = *reinterpret_cast<uint32_t*>(&exe[0x30]);

    m_cpu.getCPURegs().gpr_regs[28] = initialR28;

    if (initialR29_R30 != 0) {
        m_cpu.getCPURegs().gpr_regs[29] = initialR29_R30;
        m_cpu.getCPURegs().gpr_regs[30] = initialR29_R30;
    }

    std::memcpy(m_mainRAM.data() + startExeRamAddress, 
        exe.data() + HEADER_SIZE, exeSize);

    pcRef = initialPC;
}

auto festation::PSXSystem::onFrameEnded() -> void
{
    assert(m_frameEndCallback);

    m_gpu.renderFrame();
    m_frameEndCallback();

    m_scheduler.scheduleEvent({ EventType::VBlank, CYCLES_FER_FRAME_NTSC, 
        [this]() {
            onFrameEnded();
        } });
}