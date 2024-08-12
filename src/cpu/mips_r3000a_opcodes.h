#pragma once
#include "mips_r3000a_state.h"
#include "cpu_masks_types_utils.h"

namespace festation
{
    // Load instructions
    void lb(reg_t rt, reg_t rs, immed16_t imm);
    void lbu(reg_t rt, reg_t rs, immed16_t imm);
    void lh(reg_t rt, reg_t rs, immed16_t imm);
    void lhu(reg_t rt, reg_t rs, immed16_t imm);
    void lw(reg_t rt, reg_t rs, immed16_t imm);

    // Store instructions
    void sb(reg_t rt, reg_t rs, immed16_t imm);
    void sh(reg_t rt, reg_t rs, immed16_t imm);
    void sw(reg_t rt, reg_t rs, immed16_t imm);

    // Load/Store Alignment
    void lwr(reg_t rt, reg_t rs, immed16_t imm);
    void lwl(reg_t rt, reg_t rs, immed16_t imm);
    void swr(reg_t rt, reg_t rs, immed16_t imm);
    void swl(reg_t rt, reg_t rs, immed16_t imm);

    // Arithmetic instructions
    void add(reg_t rd, reg_t rs, reg_t rt);
    void addu(reg_t rd, reg_t rs, reg_t rt);
    void sub(reg_t rd, reg_t rs, reg_t rt);
    void subu(reg_t rd, reg_t rs, reg_t rt);
    void addi(reg_t rt, reg_t rs, immed16_t imm);
    void addiu(reg_t rt, reg_t rs, immed16_t imm);

    // Comparison instructions
    void slt(reg_t rd, reg_t rs, reg_t rt);
    void sltu(reg_t rd, reg_t rs, reg_t rt);
    void slti(reg_t rt, reg_t rs, immed16_t imm);
    void sltiu(reg_t rt, reg_t rs, immed16_t imm);

    // Logical instructions
    void _and(reg_t rd, reg_t rs, reg_t rt);
    void _or(reg_t rd, reg_t rs, reg_t rt);
    void _xor(reg_t rd, reg_t rs, reg_t rt);
    void nor(reg_t rd, reg_t rs, reg_t rt);
    void andi(reg_t rt, reg_t rs, immed16_t imm);
    void ori(reg_t rt, reg_t rs, immed16_t imm);
    void xori(reg_t rt, reg_t rs, immed16_t imm);

    // Shifting instructions
    void sllv(reg_t rd, reg_t rt, reg_t rs);
    void srlv(reg_t rd, reg_t rt, reg_t rs);
    void srav(reg_t rd, reg_t rt, reg_t rs);
    void sll(reg_t rd, reg_t rt, shift_t shift_imm);
    void srl(reg_t rd, reg_t rt, shift_t shift_imm);
    void sra(reg_t rd, reg_t rt, shift_t shift_imm);
    void lui(reg_t rt, immed16_t imm);

    // Multiply/Divide instructions
    void mult(reg_t rs, reg_t rt);
    void multu(reg_t rs, reg_t rt);
    void div(reg_t rs, reg_t rt);
    void divu(reg_t rs, reg_t rt);
    void mfhi(reg_t rd);
    void mflo(reg_t rd);
    void mthi(reg_t rs);
    void mtlo(reg_t rs);

    // Jump and Branches instructions
    void j(j_immed26_t dest);
    void jal(j_immed26_t dest);
    void jr(reg_t rs);
    void jalr(reg_t rs);
    void beq(reg_t rs, reg_t rt, immed16_t dest);
    void bne(reg_t rs, reg_t rt, immed16_t dest);
    void bltz(reg_t rs, immed16_t dest);
    void bgez(reg_t rs, immed16_t dest);
    void bgtz(reg_t rs, immed16_t dest);
    void blez(reg_t rs, immed16_t dest);
    void bltzal(reg_t rs, immed16_t dest);
    void bgezal(reg_t rs, immed16_t dest);

    // Exceptions opcodes
    void syscall(uint32_t imm20);
    void _break(uint32_t imm20);
};
