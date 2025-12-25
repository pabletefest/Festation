#pragma once
#include "cpu_masks_types_utils.hpp"

namespace festation
{
    // Load instructions
    void lb(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);
    void lbu(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);
    void lh(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);
    void lhu(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);
    void lw(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);

    // Store instructions
    void sb(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);
    void sh(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);
    void sw(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);

    // Load/Store Alignment
    void lwr(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);
    void lwl(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);
    void swr(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);
    void swl(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);

    // Arithmetic instructions
    void add(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt);
    void addu(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt);
    void sub(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt);
    void subu(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt);
    void addi(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);
    void addiu(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);

    // Comparison instructions
    void slt(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt);
    void sltu(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt);
    void slti(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);
    void sltiu(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);

    // Logical instructions
    void _and(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt);
    void _or(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt);
    void _xor(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt);
    void nor(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt);
    void andi(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);
    void ori(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);
    void xori(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm);

    // Shifting instructions
    void sllv(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rt, reg_t rs);
    void srlv(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rt, reg_t rs);
    void srav(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rt, reg_t rs);
    void sll(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rt, shift_t shift_imm);
    void srl(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rt, shift_t shift_imm);
    void sra(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rt, shift_t shift_imm);
    void lui(MIPS_R3000A_Core& cpu, reg_t rt, immed16_t imm);

    // Multiply/Divide instructions
    void mult(MIPS_R3000A_Core& cpu, reg_t rs, reg_t rt);
    void multu(MIPS_R3000A_Core& cpu, reg_t rs, reg_t rt);
    void div(MIPS_R3000A_Core& cpu, reg_t rs, reg_t rt);
    void divu(MIPS_R3000A_Core& cpu, reg_t rs, reg_t rt);
    void mfhi(MIPS_R3000A_Core& cpu, reg_t rd);
    void mflo(MIPS_R3000A_Core& cpu, reg_t rd);
    void mthi(MIPS_R3000A_Core& cpu, reg_t rs);
    void mtlo(MIPS_R3000A_Core& cpu, reg_t rs);

    // Jump and Branches instructions
    void j(MIPS_R3000A_Core& cpu, j_immed26_t dest);
    void jal(MIPS_R3000A_Core& cpu, j_immed26_t dest);
    void jr(MIPS_R3000A_Core& cpu, reg_t rs);
    void jalr(MIPS_R3000A_Core& cpu, reg_t rs, reg_t rd);
    void beq(MIPS_R3000A_Core& cpu, reg_t rs, reg_t rt, immed16_t dest);
    void bne(MIPS_R3000A_Core& cpu, reg_t rs, reg_t rt, immed16_t dest);
    void bltz(MIPS_R3000A_Core& cpu, reg_t rs, immed16_t dest);
    void bgez(MIPS_R3000A_Core& cpu, reg_t rs, immed16_t dest);
    void bgtz(MIPS_R3000A_Core& cpu, reg_t rs, immed16_t dest);
    void blez(MIPS_R3000A_Core& cpu, reg_t rs, immed16_t dest);
    void bltzal(MIPS_R3000A_Core& cpu, reg_t rs, immed16_t dest);
    void bgezal(MIPS_R3000A_Core& cpu, reg_t rs, immed16_t dest);

    // Exceptions opcodes
    void syscall(MIPS_R3000A_Core& cpu, uint32_t imm20);
    void _break(MIPS_R3000A_Core& cpu, uint32_t imm20);
};
