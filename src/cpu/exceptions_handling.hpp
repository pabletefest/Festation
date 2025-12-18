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

    void setExceptionExcodeOnRegCAUSE(MIPS_R3000A_Core& cpu, COP0ExeptionExcodes excode, bool isInterrupt);
    bool handleAndSetBadVaddrReg(MIPS_R3000A_Core& cpu, uint32_t badAddr, uint8_t boundary);
    void jumpToExceptionVector(MIPS_R3000A_Core& cpu, ExceptionVectorType exceptionVectorType);
    void handleReset(MIPS_R3000A_Core& cpu);
};
