#include "exceptions_handling.hpp"
#include "psx_cw33300_cpu.hpp"
#include "utils/logger.hpp"

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

    static void jumpToExceptionVector(MIPS_R3000A_Core& cpu, ExceptionVectorType exceptionVectorType)
    {
        switch (exceptionVectorType)
        {
        case ExceptionVectorType::Reset:
            cpu.getCPURegs().pc = (cpu.getCOP0Regs().SR.bev) ? Reset_BEV1 : Reset_BEV0;
            cpu.getCPURegs().currentPC = cpu.getCPURegs().pc;
            cpu.getCPURegs().nextPC = cpu.getCPURegs().pc + 4;
            break;
        case ExceptionVectorType::UTLB_Miss:
            cpu.getCPURegs().pc = (cpu.getCOP0Regs().SR.bev) ? UTLB_Miss_BEV1 : UTLB_Miss_BEV0;
            break;
        case ExceptionVectorType::COP0_Break:
            cpu.getCPURegs().pc = (cpu.getCOP0Regs().SR.bev) ? COP0_Break_BEV1 : COP0_Break_BEV0;
            break;
        case ExceptionVectorType::General:
            cpu.getCPURegs().pc = (cpu.getCOP0Regs().SR.bev) ? General_BEV1 : General_BEV0;
            cpu.getCPURegs().nextPC = cpu.getCPURegs().pc + 4;
            break;
        default:
            break;
        }
    }

    void handleException(MIPS_R3000A_Core& cpu, COP0ExceptionCodes excCode)
    {
        const uint32_t cause = cpu.getCOP0Regs().CAUSE.r;
        uint32_t sr = cpu.getCOP0Regs().SR.r;

        // Disable current interrrupt enable
        uint8_t srInterruptBits = sr & 0x3F;
        sr &= ~0x3F;
        sr |= ((srInterruptBits << 2) & 0x3F);

        cpu.getCOP0Regs().SR.r |= sr;

        //LOG_DEBUG("New SR COP0 reg value is 0x{:08X}", cpu.getCOP0Regs().SR);

        /* Move this later to interrupts module */
        bool isInterrupt = false;

        if ((cause & 0xFF00) && (sr & 0xFF00) && (sr & 1)
            && excCode == ExcCode_INT) {
            isInterrupt = true;
        }
        /* ------------------------------------ */

        uint32_t address = cpu.getCPURegs().currentPC;

        if (cpu.getCPURegs().isDelaySlot) {
            cpu.getCOP0Regs().CAUSE.bd = 1;
            address -= 4;
        }
        else {
            cpu.getCOP0Regs().CAUSE.bd = 0;
        }
            
        cpu.getCOP0Regs().EPC = address;

        //LOG_DEBUG("New EPC COP0 reg value is 0x{:08X}", cpu.getCOP0Regs().EPC);

        cpu.getCOP0Regs().CAUSE.excCode = excCode;
        // cpu.getCOP0Regs().CAUSE.r = excCode << 2;
        //LOG_DEBUG("New CAUSE COP0 reg value is 0x{:08X}", cpu.getCOP0Regs().CAUSE);

        jumpToExceptionVector(cpu, ExceptionVectorType::General);
    }

    bool handleAndSetBadVaddrReg(MIPS_R3000A_Core& cpu, uint32_t badAddr, AddressBoundary boundary)
    {
        assert((boundary == AddressBoundary::HALF_WORD_BOUNDARY || boundary == AddressBoundary::WORD_BOUNDARY) 
            && "Boundary error when checking for misaligned address exceptions, boundary is not 2 nor 4!");

        // We don't take into account MMU exceptions (not present on PS1) and outside kuseg in User mode (PS1 always runs kernel mode)
        if (badAddr % static_cast<uint32_t>(boundary) != 0)
        {
            cpu.getCOP0Regs().BadVaddr = badAddr;
            return true;
        }

        return false;
    }



    void handleReset(MIPS_R3000A_Core& cpu)
    {
        // For a Playstation with CXD8606CQ CPU, the PRID value is 00000002h. (psx-spx)
        cpu.getCOP0Regs().PRID = 0x00000002;

        // Reason unknown for the moment but it appears to be the initial register value.
        cpu.getCOP0Regs().SR.r = 0x10900000;

        jumpToExceptionVector(cpu, ExceptionVectorType::Reset);
    }
};
