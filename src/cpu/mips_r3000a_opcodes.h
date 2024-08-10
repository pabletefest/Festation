#pragma once
#include "mips_r3000a_state.h"

namespace festation
{
    extern PSXRegs r3000a_regs;

    // Load instructions
    void lb(uint8_t rt, uint8_t rs, uint16_t imm);
    void lbu(uint8_t rt, uint8_t rs, uint16_t imm);
    void lh(uint8_t rt, uint8_t rs, uint16_t imm);
    void lhu(uint8_t rt, uint8_t rs, uint16_t imm);
    void lw(uint8_t rt, uint8_t rs, uint16_t imm);

    // Store instructions
    void sb(uint8_t rt, uint8_t rs, uint16_t imm);
    void sh(uint8_t rt, uint8_t rs, uint16_t imm);
    void sw(uint8_t rt, uint8_t rs, uint16_t imm);

    // Load/Store Alignment
    void lwr(uint8_t rt, uint8_t rs, uint16_t imm);
    void lwl(uint8_t rt, uint8_t rs, uint16_t imm);
    void swr(uint8_t rt, uint8_t rs, uint16_t imm);
    void swl(uint8_t rt, uint8_t rs, uint16_t imm);

    // Arithmetic instructions
    void add(uint8_t rd, uint8_t rs, uint8_t rt);
    void addu(uint8_t rd, uint8_t rs, uint8_t rt);
    void sub(uint8_t rd, uint8_t rs, uint8_t rt);
    void subu(uint8_t rd, uint8_t rs, uint8_t rt);
    void addi(uint8_t rt, uint8_t rs, uint16_t imm);
    void addiu(uint8_t rt, uint8_t rs, uint16_t imm);

    // Comparison instructions
    void slt(uint8_t rd, uint8_t rs, uint8_t rt);
    void sltu(uint8_t rd, uint8_t rs, uint8_t rt);
    void slti(uint8_t rt, uint8_t rs, uint16_t imm);
    void sltiu(uint8_t rt, uint8_t rs, uint16_t imm);

    // Logical instructions
    void _and(uint8_t rd, uint8_t rs, uint8_t rt);
    void _or(uint8_t rd, uint8_t rs, uint8_t rt);
    void _xor(uint8_t rd, uint8_t rs, uint8_t rt);
    void nor(uint8_t rd, uint8_t rs, uint8_t rt);
    void andi(uint8_t rt, uint8_t rs, uint16_t imm);
    void ori(uint8_t rt, uint8_t rs, uint16_t imm);
    void xori(uint8_t rt, uint8_t rs, uint16_t imm);

    // Shifting instructions
    void sllv(uint8_t rd, uint8_t rt, uint8_t rs);
    void srlv(uint8_t rd, uint8_t rt, uint8_t rs);
    void srav(uint8_t rd, uint8_t rt, uint8_t rs);
    void sll(uint8_t rd, uint8_t rt, uint8_t shift_imm);
    void srl(uint8_t rd, uint8_t rt, uint8_t shift_imm);
    void sra(uint8_t rd, uint8_t rt, uint8_t shift_imm);
    void lui(uint8_t rt, uint16_t imm);

    // Multiply/Divide instructions
    void mult(uint8_t rs, uint8_t rt);
    void multu(uint8_t rs, uint8_t rt);
    void div(uint8_t rs, uint8_t rt);
    void divu(uint8_t rs, uint8_t rt);
    void mfhi(uint8_t rd);
    void mflo(uint8_t rd);
    void mthi(uint8_t rs);
    void mtlo(uint8_t rs);

    // Jump and Branches instructions
    void j(uint32_t dest);
    void jal(uint32_t dest);
    void jr(uint8_t rs);
    void jalr(uint8_t rs);
    void beq(uint8_t rs, uint8_t rt, uint16_t dest);
    void bne(uint8_t rs, uint8_t rt, uint16_t dest);
    void bltz(uint8_t rs, uint16_t dest);
    void bgez(uint8_t rs, uint16_t dest);
    void bgtz(uint8_t rs, uint16_t dest);
    void blez(uint8_t rs, uint16_t dest);
    void bltzal(uint8_t rs, uint16_t dest);
    void bgezal(uint8_t rs, uint16_t dest);

    // Exceptions opcodes
    void syscall(uint32_t imm20);
    void _break(uint32_t imm20);
};
