#pragma once
#include "psx_cpu.h"

namespace festation
{
    class PSXSystem
    {
    public:
        PSXSystem() = default;
        ~PSXSystem() = default;

        uint8_t read8(uint32_t address);
        uint16_t read16(uint32_t address);
        uint32_t read32(uint32_t address);
        
        void write8(uint32_t address, uint8_t value);
        void write16(uint32_t address, uint16_t value);
        void write32(uint32_t address, uint32_t value);

        void runWholeFrame();

    private:
        MIPS_R3000A cpu;
        uint8_t* mainRAM = nullptr;
    };
};
