#pragma once
#include "cpu_masks_types_utils.hpp"


namespace festation
{
    class MIPS_R3000A_Core;

    void mfc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rd);                  // ;rt = cop#datRd ;data regs
    void cfc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rd);                  // ;rt = cop#cntRd ;control regs
    void mtc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rd);                  // ;cop#datRd = rt ;data regs
    void ctc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rd);                  // ;cop#cntRd = rt ;control regs
    void cop0(MIPS_R3000A_Core& cpu, cop0_command_t imm25);                // ;exec cop# command 0..1FFFFFFh
    void lwc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);   // ;cop#dat_rt = [rs+imm]  ;word
    void swc0(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);   // ;[rs+imm] = cop#dat_rt  ;word
    void bc0f(MIPS_R3000A_Core& cpu, immed16_t dest);                      // ;if cop#flg=false then pc=$+disp
    void bc0t(MIPS_R3000A_Core& cpu, immed16_t dest);                      // ;if cop#flg=true  then pc=$+disp
    void rfe(MIPS_R3000A_Core& cpu);                                       // ;return from exception (COP0)
    // tlb<xx>                                                             // ;virtual memory related (COP0), unused in the PS1
};
