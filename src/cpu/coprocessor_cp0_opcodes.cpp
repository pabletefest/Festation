#include "coprocessor_cp0_opcodes.hpp"
#include "psx_system.hpp"
#include "psx_cpu_state.hpp"
#include "cpu_masks_types_utils.hpp"

namespace festation
{
    void mfc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rd)
    {
        if (cpu.getCPURegs().isLoadDelaySlot()) {
            cpu.getCPURegs().consumeLoadedData();
        }

        cpu.getCPURegs().gpr_regs[rt] = cpu.getCOP0Regs().getCop0RegisterValue(rd);
        //cpu.getCPURegs().storeDelayedData(cpu.getCOP0Regs().getCop0RegisterValue(rd), rt);
    }

    void cfc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rd)
    {
        // Do not apply for COP0
    }

    void mtc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rd)
    {
        if (cpu.getCPURegs().isLoadDelaySlot()) {
            cpu.getCPURegs().consumeLoadedData();
        }

        cpu.getCOP0Regs().setCop0RegisterValue(rd, cpu.getCPURegs().gpr_regs[rt]);
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
        if (cpu.getCPURegs().isLoadDelaySlot()) {
            cpu.getCPURegs().consumeLoadedData();
        }

        cpu.getCOP0Regs().setCop0RegisterValue(rt, cpu.read32(cpu.getCPURegs().gpr_regs[rs] + signExtend(imm)));
    }

    void swc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
        if (cpu.getCPURegs().isLoadDelaySlot()) {
            cpu.getCPURegs().consumeLoadedData();
        }

        cpu.write32(cpu.getCPURegs().gpr_regs[rs] + signExtend(imm), cpu.getCOP0Regs().getCop0RegisterValue(rt));
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
        uint32_t shiftedBitsSR = cpu.getCOP0Regs().SR;

        if (cpu.getCPURegs().isLoadDelaySlot()) {
            cpu.getCPURegs().consumeLoadedData();
        }

        shiftedBitsSR >>= 2;

        cpu.getCOP0Regs().SR = (cpu.getCOP0Regs().SR & 0xFFFFFFF0) | (shiftedBitsSR & 0xFu);
    }
}
