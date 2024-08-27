#include "exceptions_handling.hpp"
#include "psx_cpu_state.hpp"

namespace festation
{
    extern PSXRegs r3000a_regs;
    extern CP0SystemControlRegs cp0_state;

    static void setEPCReg(bool isInterrupt)
    {
        uint32_t address = r3000a_regs.pc;

        if ((cp0_state.cp0_regs[CAUSE] & 0x80000000) && !isInterrupt)
            address -= 4;

        cp0_state.cp0_regs[EPC] = address;
    }

    void setExceptionExcodeOnRegCAUSE(COP0ExeptionExcodes excode)
    {
        cp0_state.cp0_regs[CAUSE] = (cp0_state.cp0_regs[CAUSE] & 0xFFFFFF00) | (excode << 2);
    }
};
