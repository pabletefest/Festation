#include "coprocessor_cp0_opcodes.hpp"
#include "psx_system.hpp"
#include "psx_cpu_state.hpp"

namespace festation
{
    void mfc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rd)
    {
        //cpu.getCPURegs().gpr_regs[rt] = cpu.getCOP0Regs().cop0_regs[rd];
        cpu.getCPURegs().storeDelayedData(cpu.getCOP0Regs().cop0_regs[rd], rt);
    }

    void cfc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rd)
    {
        // Do not apply for COP0
    }

    void mtc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rd)
    {
        cpu.getCOP0Regs().cop0_regs[rd] = cpu.getCPURegs().gpr_regs[rt];
    }
    
    void ctc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rd)
    {
        // Do not apply for COP0
    }
    
    void cop0(MIPS_R3000A_Core& cpu, cop0_command_t imm25)
    {
        // Do not apply for COP0       
    }

    void lwc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
        cpu.getCOP0Regs().cop0_regs[rt] = cpu.read32(cpu.getCPURegs().gpr_regs[rs] + imm);
    }

    void swc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
        cpu.write32(cpu.getCPURegs().gpr_regs[rs] + imm, cpu.getCOP0Regs().cop0_regs[rt]);
    }

    void bc0f(MIPS_R3000A_Core& cpu, immed16_t dest)
    {
        // Do not apply for COP0
    }
    
    void bc0t(MIPS_R3000A_Core& cpu, immed16_t dest)
    {
        // Do not apply for COP0
    }

    void rfe(MIPS_R3000A_Core& cpu)
    {
        uint32_t shiftedBitsSR = cpu.getCOP0Regs().cop0_regs[SR];

        shiftedBitsSR >>= 2;

        cpu.getCOP0Regs().cop0_regs[SR] = (cpu.getCOP0Regs().cop0_regs[SR] & 0xFFFFFFF0) | (shiftedBitsSR & 0xFu);
    }
}
