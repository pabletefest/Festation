#pragma once

#include "dma/dma_channel.hpp"

#include <cstdint>
#include <array>

namespace festation {
    inline constexpr uint8_t DMA_CHANNELS = 7u;

    class DmaControl {
    public:
        DmaControl();
        ~DmaControl();

        void reset();

        uint32_t read32(uint32_t address);
        void write32(uint32_t address, uint32_t value);

    private:
        union DmaControlRegister {
            struct {
                uint32_t mdecInPriority : 3;
                uint32_t mdecInMasterEnable : 1;
                uint32_t mdecOutPriority : 3;
                uint32_t mdecOutMasterEnable : 1;
                uint32_t gpuPriority : 3;
                uint32_t gpuMasterEnable : 1;
                uint32_t cdromPriority : 3;
                uint32_t cdromMasterEnable : 1;
                uint32_t spuPriority : 3;
                uint32_t spuMasterEnable : 1;
                uint32_t pioPriority : 3;
                uint32_t pioMasterEnable : 1;
                uint32_t otcPriority : 3;
                uint32_t otcMasterEnable : 1;
                uint32_t cpuMemoryAccessPriority : 3;
                uint32_t cpuMemoryAccessEnable : 1; // Docs say: No effect, should be CPU memory access enable (R/W)
            };

            uint32_t raw;
        } DPCR;

        union DmaInterruptRegister {
            struct {
                uint32_t channelsCompletionInterruptsControl : 7;
                uint32_t unused : 8;
                uint32_t busErrorFlag : 1;
                uint32_t channelsInterruptMask : 7;
                uint32_t masterChannelInterruptEnable : 1;
                uint32_t channelsInterruptFlags : 7;
                uint32_t masterInterruptFlag : 1;
            };
            
            uint32_t raw;
        } DICR;

        std::array<DmaChannel, DMA_CHANNELS> channels;
    };
};