#pragma once
#include "cpu_masks_types_utils.hpp"

namespace festation
{
    enum class ExceptionVectorType
    {
        Reset,      // (Reset)
        UTLB_Miss,  // (Virtual memory, none such in PSX)
        COP0_Break, // (Debug Break)
        General     // (General Interrupts & Exceptions)
    };

    enum class AddressBoundary {
        HALF_WORD_BOUNDARY = 2,
        WORD_BOUNDARY = 4
    };

    void handleException(MIPS_R3000A_Core& cpu, COP0ExeptionCodes excCode);
    bool handleAndSetBadVaddrReg(MIPS_R3000A_Core& cpu, uint32_t badAddr, AddressBoundary boundary);
    void handleReset(MIPS_R3000A_Core& cpu);
};
