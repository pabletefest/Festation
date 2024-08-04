#pragma once
#include "mips_r300a_state.h"

#include <cstdint>

namespace festation
{
    class MIPS_R3000A
    {
    public:
        MIPS_R3000A() = default;
        ~MIPS_R3000A() = default;

        void clockCycles(uint32_t cycles);

    private:
        uint64_t totalCyclesElapsed;

        PSXRegs r3000a_regs;
    };
};
