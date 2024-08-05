#pragma once
#include "mips_r300a_state.h"

#include <cstdint>

namespace festation
{
    static constexpr float CPU_CLOCK_SPEED = 33.8688f; // MHz
    static constexpr uint32_t CPU_CLOCKS_PER_SECOND = 33'868'800;

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
