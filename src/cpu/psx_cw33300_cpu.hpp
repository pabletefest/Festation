#pragma once
#include "psx_cpu_state.hpp"
#include "cpu_masks_types_utils.hpp"

#include <cstdint>
#include <array>

namespace festation
{
    class PSXSystem;

    static constexpr float CPU_CLOCK_SPEED = 33.8688f; // MHz
    static constexpr uint32_t CPU_CLOCKS_PER_SECOND = 33'868'800;

    class MIPS_R3000A_Core
    {
    public:
        MIPS_R3000A_Core(PSXSystem* device);
        ~MIPS_R3000A_Core() = default;

        void reset();

        uint8_t read8(uint32_t address);
        uint16_t read16(uint32_t address);
        uint32_t read32(uint32_t address);
        
        void write8(uint32_t address, uint8_t value);
        void write16(uint32_t address, uint16_t value);
        void write32(uint32_t address, uint32_t value);

        void executeInstruction();
        void clockCycles(uint32_t cycles);

        PSXRegs& getCPURegs();
        COP0SystemControlRegs& getCOP0Regs();

    private:        
        uint32_t fetchInstruction();
        InstructionType decodeInstruction(uint32_t instruction);
        InstructionTypeVariant decodeRFormat(uint32_t instruction);
        InstructionTypeVariant decodeJFormat(uint32_t instruction);
        InstructionTypeVariant decodeIFormat(uint32_t instruction);

    private:
        uint64_t totalCyclesElapsed;
        PSXSystem* system = nullptr;

        PSXRegs r3000a_regs;
        COP0SystemControlRegs cop0_state;

        std::array<uint8_t, 1024> scratchpadCache;
    };
};
