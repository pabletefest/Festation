#include "coprocessor_cp0_opcodes.hpp"
#include "psx_system.hpp"
#include "psx_cpu_state.hpp"

namespace festation
{
    extern PSXSystem psxSystem;
    extern PSXRegs r3000a_regs;
    extern CP0SystemControlRegs cp0_state;

    void mfc0(reg_t rt, reg_t rd)
    {
        r3000a_regs.gpr_regs[rt] = cp0_state.cp0_regs[rd];
    }

    void cfc0(reg_t rt, reg_t rd)
    {
        // Do not apply for COP0
    }

    void mtc0(reg_t rt, reg_t rd)
    {
        cp0_state.cp0_regs[rd] = r3000a_regs.gpr_regs[rt];
    }
    
    void ctc0(reg_t rt, reg_t rd)
    {
        // Do not apply for COP0
    }
    
    void cop0(cop0_command_t imm25)
    {
        // Do not apply for COP0       
    }

    void lwc0(reg_t rt, reg_t rs, immed16_t imm)
    {
        cp0_state.cp0_regs[rt] = psxSystem.read32(r3000a_regs.gpr_regs[rs] + imm); 
    }

    void swc0(reg_t rt, reg_t rs, immed16_t imm)
    {
        psxSystem.write32(r3000a_regs.gpr_regs[rs] + imm, cp0_state.cp0_regs[rt]);
    }

    void bc0f(immed16_t dest)
    {
        // Do not apply for COP0
    }
    
    void bc0t(immed16_t dest)
    {
        // Do not apply for COP0
    }

    void rfe()
    {
        uint32_t shiftedBitsSR = cp0_state.cp0_regs[SR];

        shiftedBitsSR >>= 2;

        cp0_state.cp0_regs[SR] = (cp0_state.cp0_regs[SR] & 0xFFFFFFF0) | (shiftedBitsSR & 0xFu);
    }
}
