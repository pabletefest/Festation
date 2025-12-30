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

    enum COP0ExceptionCodes
    {
        ExcCode_INT,
        ExcCode_MOD,
        ExcCode_TLBL,
        ExcCode_TLBS,
        ExcCode_AdEL,
        ExcCode_AdES,
        ExcCode_IBE,
        ExcCode_DBE,
        ExcCode_Syscall,
        ExcCode_BP,
        ExcCode_RI,
        ExcCode_CpU,
        ExcCode_Ov,
        ExcCode_UNUSED
    };

    void handleException(MIPS_R3000A_Core& cpu, COP0ExceptionCodes excCode);
    bool handleAndSetBadVaddrReg(MIPS_R3000A_Core& cpu, uint32_t badAddr, AddressBoundary boundary);
    void handleReset(MIPS_R3000A_Core& cpu);
};
