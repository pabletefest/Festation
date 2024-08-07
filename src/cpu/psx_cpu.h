#pragma once
#include "mips_r3000a_state.h"
#include "cpu_masks_types_utils.h"

#include <cstdint>

// TEMP
#include <vector>

namespace festation
{
    static constexpr float CPU_CLOCK_SPEED = 33.8688f; // MHz
    static constexpr uint32_t CPU_CLOCKS_PER_SECOND = 33'868'800;

    class MIPS_R3000A
    {
    public:
        MIPS_R3000A();
        ~MIPS_R3000A() = default;

        uint8_t read8(uint32_t address);
        uint16_t read16(uint32_t address);
        uint32_t read32(uint32_t address);
        
        void write8(uint32_t address, uint8_t value);
        void write16(uint32_t address, uint16_t value);
        void write32(uint32_t address, uint32_t value);

        void executeInstruction();
        void clockCycles(uint32_t cycles);

    private:        
        uint32_t fetchInstruction();
        InstructionType decodeInstruction(uint32_t instruction);
        InstructionTypePtr decodeRFormat(uint32_t instruction);
        InstructionTypePtr decodeJFormat(uint32_t instruction);
        InstructionTypePtr decodeIFormat(uint32_t instruction);

    private:
        uint64_t totalCyclesElapsed;

        // TEMP
        std::vector<uint8_t> tempRAM;
    };
};
