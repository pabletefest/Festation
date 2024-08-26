#pragma once
#include "cpu_masks_types_utils.hpp"


namespace festation
{
    void mfc0(reg_t rt, reg_t rd);                  // ;rt = cop#datRd ;data regs
    void cfc0(reg_t rt, reg_t rd);                  // ;rt = cop#cntRd ;control regs
    void mtc0(reg_t rt, reg_t rd);                  // ;cop#datRd = rt ;data regs
    void ctc0(reg_t rt, reg_t rd);                  // ;cop#cntRd = rt ;control regs
    void cop0(cop0_command_t imm25);                // ;exec cop# command 0..1FFFFFFh
    void lwc0(reg_t rt, reg_t rs, immed16_t imm);   // ;cop#dat_rt = [rs+imm]  ;word
    void swc0(reg_t rt, reg_t rs, immed16_t imm);   // ;[rs+imm] = cop#dat_rt  ;word
    void bc0f(immed16_t dest);                      // ;if cop#flg=false then pc=$+disp
    void bc0t(immed16_t dest);                      // ;if cop#flg=true  then pc=$+disp
    void rfe();                                     // ;return from exception (COP0)
    // tlb<xx>                                      // ;virtual memory related (COP0), unused in the PS1
};
