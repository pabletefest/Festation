#pragma once            

#include <cstdint>
#include <utility>

namespace festation
{
    struct PSXRegs
    {
        uint32_t gpr_regs[32];  // General Porpouse Registers
        uint32_t pc;            // Program Counter
        uint32_t hi;            // High part of mult/div opcodes results
        uint32_t lo;            // Low part of mult/div opcodes results

        std::pair<bool, uint32_t> delaySlotLatch; // Pair containing last latched loaded value from memory due to delay slot (cleared after being consumed)
    };
};
