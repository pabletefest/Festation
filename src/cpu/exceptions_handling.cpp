#include "exceptions_handling.hpp"
#include "psx_cw33300_cpu.hpp"

#include <cassert>

namespace festation
{
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


    static void setEPCReg(MIPS_R3000A_Core& cpu, bool isInterrupt)
    {
        uint32_t address = cpu.getCPURegs().pc;

        if ((cpu.getCOP0Regs().cop0_regs[CAUSE] & 0x80000000) && !isInterrupt)
            address -= 4;

        cpu.getCOP0Regs().cop0_regs[EPC] = address;
    }

    void setExceptionExcodeOnRegCAUSE(MIPS_R3000A_Core& cpu, COP0ExeptionExcodes excode, bool isInterrupt)
    {
        cpu.getCOP0Regs().cop0_regs[CAUSE] = (cpu.getCOP0Regs().cop0_regs[CAUSE] & 0xFFFFFF00) | (excode << 2);

        setEPCReg(cpu, isInterrupt);
    }

    bool handleAndSetBadVaddrReg(MIPS_R3000A_Core& cpu, uint32_t badAddr, uint8_t boundary)
    {
        assert((boundary == 2 || boundary == 4) && "Boundary error when checking for misaligned address exceptions, boundary is not 2 nor 4!");

        // We don't take into account MMU exceptions (not present on PS1) and outside kuseg in User mode (PS1 always runs kernel mode)
        if (badAddr % boundary != 0)
        {
            cpu.getCOP0Regs().cop0_regs[BadVaddr] = badAddr;
            return true;
        }

        return false;
    }

    void jumpToExceptionVector(MIPS_R3000A_Core& cpu, ExceptionVectorType exceptionVectorType)
    {
        uint8_t BEVbit = (cpu.getCOP0Regs().cop0_regs[SR] >> 22) & 1;

        switch (exceptionVectorType)
        {
        case ExceptionVectorType::Reset:
            cpu.getCPURegs().pc = (BEVbit) ? (uint32_t)Reset_BEV1 : (uint32_t)Reset_BEV0;
            break;
        case ExceptionVectorType::UTLB_Miss:
            cpu.getCPURegs().pc = (BEVbit) ? (uint32_t)UTLB_Miss_BEV1 : (uint32_t)UTLB_Miss_BEV0;
            break;
        case ExceptionVectorType::COP0_Break:
            cpu.getCPURegs().pc = (BEVbit) ? (uint32_t)COP0_Break_BEV1 : (uint32_t)COP0_Break_BEV0;
            break;
        case ExceptionVectorType::General:
            cpu.getCPURegs().pc = (BEVbit) ? (uint32_t)General_BEV1 : (uint32_t)General_BEV0;
            break;  
        default:
            break;
        }
    }

    void handleReset(MIPS_R3000A_Core& cpu)
    {
        // For a Playstation with CXD8606CQ CPU, the PRID value is 00000002h. (psx-spx)
        cpu.getCOP0Regs().cop0_regs[PRID] = 0x00000002;

        // Reason unknown for the moment but it appears to be the initial register value.
        cpu.getCOP0Regs().cop0_regs[SR] = 0x10900000;

        jumpToExceptionVector(cpu, ExceptionVectorType::Reset);
    }
};
