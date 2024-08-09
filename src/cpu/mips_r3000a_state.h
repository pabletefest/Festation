#pragma once            

#include <cstdint>

namespace festation
{
    struct PSXRegs
    {
        uint32_t gpr_regs[32];  // General Porpouse Registers
        uint32_t pc;            // Program Counter
        uint32_t hi;            // High part of mult/div opcodes results
        uint32_t lo;            // Low part of mult/div opcodes results
    };
};
