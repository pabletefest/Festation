#pragma once
#include "cdrom/cdrom.hpp"
#include "cpu/psx_cw33300_cpu.hpp"
#include "interrupts/interrupts.hpp"
#include "kernel_bios/bios.hpp"
#include "cdrom/cdrom.hpp"
#include "dma/dma_control.hpp"
#include "gpu/gpu.hpp"
#include "scheduler/scheduler.hpp"
#include "timer/timer.hpp"

#include <vector>
#include <array>
#include <filesystem>

namespace festation
{
    class PSXSystem
    {
    public:
        PSXSystem();
        ~PSXSystem();

        auto reset() -> void;
        
        template<class Func>
        inline auto setFrameEndCallback(Func&& callback) -> void {
            this->m_frameEndCallback = callback;
        }

        auto read8(uint32_t address) -> uint8_t;
        auto read16(uint32_t address) -> uint16_t;
        auto read32(uint32_t address) -> uint32_t;
        
        auto write8(uint32_t address, uint8_t value) -> void;
        auto write16(uint32_t address, uint16_t value) -> void;
        auto write32(uint32_t address, uint32_t value) -> void;

        auto run() -> void;
        auto runWholeFrame() -> void;
        auto sideloadExeFile(const std::filesystem::path& path) -> void;

    private:
        auto onFrameEnded() -> void;

    private:
        Scheduler m_scheduler;
        InterruptsHandler m_interruptsHandler;
        MIPS_R3000A_Core m_cpu;
        std::vector<uint8_t> m_mainRAM;
        KernelBIOS m_bios;
        CdromDrive m_cdrom;
        DmaControl m_dma;
        PsxGpu m_gpu;
        std::array<Timer, 3> m_timers;
        uint64_t m_totalElapsedCycles;
        std::function<void(void)> m_frameEndCallback;
    };
};
