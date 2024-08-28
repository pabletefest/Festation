#include "exceptions_handling.hpp"
#include "psx_cpu_state.hpp"

#include <cassert>

namespace festation
{
    extern PSXRegs r3000a_regs;
    extern COP0SystemControlRegs cop0_state;

    enum ExceptionVectorsBEV0 : uint32_t
    {
        Reset_BEV0 = 0xBFC00000,
        UTLB_Miss_BEV0 = 0x80000000,
        COP0_Break_BEV0 = 0x80000040,
        General_BEV0 = 0x80000080
    };

    enum ExceptionVectorsBEV1 : uint32_t
    {
        Reset_BEV1 = 0xBFC00000,
        UTLB_Miss_BEV1 = 0xBFC00100,
        COP0_Break_BEV1 =  0xBFC00140,
        General_BEV1 =  0xBFC00180
    };


    static void setEPCReg(bool isInterrupt)
    {
        uint32_t address = r3000a_regs.pc;

        if ((cop0_state.cop0_regs[CAUSE] & 0x80000000) && !isInterrupt)
            address -= 4;

        cop0_state.cop0_regs[EPC] = address;
    }

    void setExceptionExcodeOnRegCAUSE(COP0ExeptionExcodes excode, bool isInterrupt)
    {
        cop0_state.cop0_regs[CAUSE] = (cop0_state.cop0_regs[CAUSE] & 0xFFFFFF00) | (excode << 2);

        setEPCReg(isInterrupt);
    }

    bool handleAndSetBadVaddrReg(uint32_t badAddr, uint8_t boundary)
    {
        assert((boundary != 2 || boundary != 4) && "Boundary error when checking for misaligned address exceptions, boundary is not 2 nor 4!");

        // We don't take into account MMU exceptions (not present on PS1) and outside kuseg in User mode (PS1 always runs kernel mode)
        if (badAddr % boundary != 0)
        {
            cop0_state.cop0_regs[BadVaddr] = badAddr;
            return true;
        }

        return false;
    }

    void jumpToExceptionVector(ExceptionVectorType exceptionVectorType)
    {
        uint8_t BEVbit = (cop0_state.cop0_regs[SR] >> 22) & 1;

        switch (exceptionVectorType)
        {
        case ExceptionVectorType::Reset:
            r3000a_regs.pc = (BEVbit) ? Reset_BEV1 : Reset_BEV0;
            break;
        case ExceptionVectorType::UTLB_Miss:
            r3000a_regs.pc = (BEVbit) ? UTLB_Miss_BEV1 : UTLB_Miss_BEV0;
            break;
        case ExceptionVectorType::COP0_Break:
            r3000a_regs.pc = (BEVbit) ? COP0_Break_BEV1 : COP0_Break_BEV0;
            break;
        case ExceptionVectorType::General:
            r3000a_regs.pc = (BEVbit) ? General_BEV1 : General_BEV0;
            break;  
        default:
            break;
        }
    }

    void handleReset()
    {
        // For a Playstation with CXD8606CQ CPU, the PRID value is 00000002h. (psx-spx)
        cop0_state.cop0_regs[PRID] = 0x00000002;
        
        jumpToExceptionVector(ExceptionVectorType::Reset);
    }
};
