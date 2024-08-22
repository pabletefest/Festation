#include "mips_r3000a_opcodes.hpp"
#include "psx_system.hpp"

#include <limits>

namespace festation
{
    extern PSXSystem psxSystem;
    extern PSXRegs r3000a_regs;

    // TODO: implement load delay slot 

    void lb(reg_t rt, reg_t rs, immed16_t imm)
    {
        // TODO: handle misaligned address error exceptions and invalid memory locations bus error exception

        r3000a_regs.gpr_regs[rt] = (int32_t)psxSystem.read8(r3000a_regs.gpr_regs[rs] + imm);
    }

    void lbu(reg_t rt, reg_t rs, immed16_t imm)
    {
        // TODO: handle misaligned address error exceptions and invalid memory locations bus error exception     

        r3000a_regs.gpr_regs[rt] = psxSystem.read8(r3000a_regs.gpr_regs[rs] + imm);
    }

    void lh(reg_t rt, reg_t rs, immed16_t imm)
    {
        // TODO: handle misaligned address error exceptions and invalid memory locations bus error exception

        r3000a_regs.gpr_regs[rt] = (int32_t)psxSystem.read16(r3000a_regs.gpr_regs[rs] + imm);
    }

    void lhu(reg_t rt, reg_t rs, immed16_t imm)
    {
        // TODO: handle misaligned address error exceptions and invalid memory locations bus error exception

        r3000a_regs.gpr_regs[rt] = psxSystem.read16(r3000a_regs.gpr_regs[rs] + imm);        
    }

    void lw(reg_t rt, reg_t rs, immed16_t imm)
    {
        // TODO: handle invalid memory locations bus error exception

        r3000a_regs.gpr_regs[rt] = psxSystem.read32(r3000a_regs.gpr_regs[rs] + imm);
    }

    // No need to implement write-queue behaviour for store instructions

    void sb(reg_t rt, reg_t rs, immed16_t imm)
    {
        psxSystem.write8(r3000a_regs.gpr_regs[rs] + imm, rt & 0xFF);
    }

    void sh(reg_t rt, reg_t rs, immed16_t imm)
    {
        psxSystem.write16(r3000a_regs.gpr_regs[rs] + imm, rt & 0xFFFF);
    }

    void sw(reg_t rt, reg_t rs, immed16_t imm)
    {
        psxSystem.write32(r3000a_regs.gpr_regs[rs] + imm, rt);
    }

    void lwr(reg_t rt, reg_t rs, immed16_t imm)
    {
        uint32_t prevRT = r3000a_regs.gpr_regs[rt];
        uint32_t address = r3000a_regs.gpr_regs[rs] + imm;
        uint8_t offset = address % 4;
        uint32_t loadedWord = psxSystem.read32(address);

        uint32_t properValue = loadedWord >> (8 * offset);
        prevRT &= (0xFFFFFFFFu << (8 * (4 - offset)));

        r3000a_regs.gpr_regs[rt] = prevRT | properValue;
    }

    void lwl(reg_t rt, reg_t rs, immed16_t imm)
    {
        uint32_t prevRT = r3000a_regs.gpr_regs[rt];
        uint32_t address = r3000a_regs.gpr_regs[rs] + imm;
        uint8_t offset = address % 4;
        uint32_t loadedWord = psxSystem.read32(address);

        uint32_t properValue = loadedWord >> (8 * (4 - offset));
        prevRT &= (0xFFFFFFFFu >> (8 * offset));

        r3000a_regs.gpr_regs[rt] = prevRT | properValue;
    }

    void swr(reg_t rt, reg_t rs, immed16_t imm)
    {
        uint32_t address = r3000a_regs.gpr_regs[rs] + imm;
        uint32_t prevStored = psxSystem.read32(address);
        uint8_t offset = address % 4;
        uint32_t storedRT = r3000a_regs.gpr_regs[rt];

        uint32_t properValue = storedRT >> (8 * offset);
        prevStored &= (0xFFFFFFFFu << (8 * (4 - offset)));

        psxSystem.write32(address, prevStored | properValue);
    }

    void swl(reg_t rt, reg_t rs, immed16_t imm)
    {
        uint32_t address = r3000a_regs.gpr_regs[rs] + imm;
        uint32_t prevStored = psxSystem.read32(address);
        uint8_t offset = address % 4;
        uint32_t storedRT = r3000a_regs.gpr_regs[rt];

        uint32_t properValue = storedRT >> (8 * (4 - offset));
        prevStored &= (0xFFFFFFFFu >> (8 * offset));

        psxSystem.write32(address, prevStored | properValue);
    }

    void add(reg_t rd, reg_t rs, reg_t rt)
    {
        int32_t result;

        #ifdef __GNUC__
            if (__builtin_add_overflow((int32_t)r3000a_regs.gpr_regs[rs], (int32_t)r3000a_regs.gpr_regs[rt], &result))
            {
                // TODO: Overflow Exception
                return;
            }

            r3000a_regs.gpr_regs[rd] = (uint32_t)result;
        #endif
    }

    void addu(reg_t rd, reg_t rs, reg_t rt)
    {
        r3000a_regs.gpr_regs[rd] = r3000a_regs.gpr_regs[rs] + r3000a_regs.gpr_regs[rt];
    }

    void sub(reg_t rd, reg_t rs, reg_t rt)
    {
        int32_t result;

        #ifdef __GNUC__
            if (__builtin_sub_overflow((int32_t)r3000a_regs.gpr_regs[rs], (int32_t)r3000a_regs.gpr_regs[rt], &result))
            {
                // TODO: Overflow Exception
                return;
            }

            r3000a_regs.gpr_regs[rd] = (uint32_t)result;
        #endif
    }

    void subu(reg_t rd, reg_t rs, reg_t rt)
    {
        r3000a_regs.gpr_regs[rd] = r3000a_regs.gpr_regs[rs] - r3000a_regs.gpr_regs[rt];
    }
    
    void addi(reg_t rt, reg_t rs, immed16_t imm)
    {
        int32_t result;

        #ifdef __GNUC__
            if (__builtin_add_overflow((int32_t)r3000a_regs.gpr_regs[rs], (int16_t)imm, &result))
            {
                // TODO: Overflow Exception
                return;
            }

            r3000a_regs.gpr_regs[rt] = (uint32_t)result;
        #endif
    }

    void addiu(reg_t rt, reg_t rs, immed16_t imm)
    {
        r3000a_regs.gpr_regs[rt] = r3000a_regs.gpr_regs[rs] + (int16_t)imm;
    }

    void slt(reg_t rd, reg_t rs, reg_t rt)
    {
        if (((int32_t)r3000a_regs.gpr_regs[rs]) < ((int32_t)r3000a_regs.gpr_regs[rt]))
            r3000a_regs.gpr_regs[rd] = 1;
        else
            r3000a_regs.gpr_regs[rd] = 0;
    }

    void sltu(reg_t rd, reg_t rs, reg_t rt)
    {
        if (r3000a_regs.gpr_regs[rs] < r3000a_regs.gpr_regs[rt])
            r3000a_regs.gpr_regs[rd] = 1;
        else
            r3000a_regs.gpr_regs[rd] = 0;
    }

    void slti(reg_t rt, reg_t rs, immed16_t imm)
    {
        if (((int32_t)r3000a_regs.gpr_regs[rs]) < ((int32_t)imm))
            r3000a_regs.gpr_regs[rt] = 1;
        else
            r3000a_regs.gpr_regs[rt] = 0;
    }

    void sltiu(reg_t rt, reg_t rs, immed16_t imm)
    {
        if (r3000a_regs.gpr_regs[rs] < (uint32_t)((int32_t)imm))
            r3000a_regs.gpr_regs[rt] = 1;
        else
            r3000a_regs.gpr_regs[rt] = 0;
    }

    void _and(reg_t rd, reg_t rs, reg_t rt)
    {
        r3000a_regs.gpr_regs[rd] = r3000a_regs.gpr_regs[rs] & r3000a_regs.gpr_regs[rt];
    }

    void _or(reg_t rd, reg_t rs, reg_t rt)
    {
        r3000a_regs.gpr_regs[rd] = r3000a_regs.gpr_regs[rs] | r3000a_regs.gpr_regs[rt];
    }

    void _xor(reg_t rd, reg_t rs, reg_t rt)
    {
        r3000a_regs.gpr_regs[rd] = r3000a_regs.gpr_regs[rs] ^ r3000a_regs.gpr_regs[rt];
    }

    void nor(reg_t rd, reg_t rs, reg_t rt)
    {
        r3000a_regs.gpr_regs[rd] = 0xFFFFFFFF ^ (r3000a_regs.gpr_regs[rs] | r3000a_regs.gpr_regs[rt]);
    }

    void andi(reg_t rt, reg_t rs, immed16_t imm)
    {
        r3000a_regs.gpr_regs[rt] = r3000a_regs.gpr_regs[rs] & imm;
    }

    void ori(reg_t rt, reg_t rs, immed16_t imm)
    {
        r3000a_regs.gpr_regs[rt] = r3000a_regs.gpr_regs[rs] | imm;
    }

    void xori(reg_t rt, reg_t rs, immed16_t imm)
    {
        r3000a_regs.gpr_regs[rt] = r3000a_regs.gpr_regs[rs] ^ imm;
    }

    void sllv(reg_t rd, reg_t rt, reg_t rs)
    {
        r3000a_regs.gpr_regs[rd] = r3000a_regs.gpr_regs[rt] << (r3000a_regs.gpr_regs[rs] & 0x1F);
    }

    void srlv(reg_t rd, reg_t rt, reg_t rs)
    {
        r3000a_regs.gpr_regs[rd] = r3000a_regs.gpr_regs[rt] >> (r3000a_regs.gpr_regs[rs] & 0x1F);
    }

    void srav(reg_t rd, reg_t rt, reg_t rs)
    {
        r3000a_regs.gpr_regs[rd] = (int32_t)r3000a_regs.gpr_regs[rt] >> (int8_t)(r3000a_regs.gpr_regs[rs] & 0x1F);   
    }

    void sll(reg_t rd, reg_t rt, shift_t imm)
    {
        r3000a_regs.gpr_regs[rd] = r3000a_regs.gpr_regs[rt] << imm;
    }

    void srl(reg_t rd, reg_t rt, shift_t imm)
    {
        r3000a_regs.gpr_regs[rd] = r3000a_regs.gpr_regs[rt] >> imm;
    }

    void sra(reg_t rd, reg_t rt, shift_t imm)
    {
        r3000a_regs.gpr_regs[rd] = (int32_t)r3000a_regs.gpr_regs[rt] >> (int8_t)imm;
    }

    void lui(reg_t rt, immed16_t imm)
    {
        r3000a_regs.gpr_regs[rt] = imm << 16;
    }

    void mult(reg_t rs, reg_t rt)
    {
        int64_t result = (int64_t)(int32_t)r3000a_regs.gpr_regs[rs] * (int64_t)(int32_t)r3000a_regs.gpr_regs[rt];

        r3000a_regs.hi = (uint64_t)(result >> 32) & 0xFFFFFFFF;
        r3000a_regs.lo = (uint64_t)result & 0xFFFFFFFF;
    }

    void multu(reg_t rs, reg_t rt)
    {
        uint64_t result = (uint64_t)r3000a_regs.gpr_regs[rs] * (uint64_t)r3000a_regs.gpr_regs[rt];

        r3000a_regs.hi = (result >> 32) & 0xFFFFFFFF;
        r3000a_regs.lo = result & 0xFFFFFFFF;
    }

    void div(reg_t rs, reg_t rt)
    {
        int32_t rsOperand = (int32_t)r3000a_regs.gpr_regs[rs];
        int32_t rtOperand = (int32_t)r3000a_regs.gpr_regs[rt];

        if (rtOperand == 0)
        {
            r3000a_regs.hi = rsOperand;

            if (rsOperand >= 0)
                r3000a_regs.lo = -1;
            else
                r3000a_regs.lo = 1;

            return;
        }
        else if (rtOperand == -1 && rsOperand == std::numeric_limits<std::int32_t>::min())
        {
            r3000a_regs.hi = 0;
            r3000a_regs.lo = rsOperand; // std::numeric_limits<std::int32_t>::min() or (int32_t)0x80000000 or -0x80000000
            
            return;
        }

        r3000a_regs.lo = (uint32_t)(rsOperand / rtOperand);
        r3000a_regs.hi = (uint32_t)(rsOperand % rtOperand);
    }

    void divu(reg_t rs, reg_t rt)
    {
        uint32_t rsOperand = r3000a_regs.gpr_regs[rs];
        uint32_t rtOperand = r3000a_regs.gpr_regs[rt];

        if (rtOperand == 0)
        {
            r3000a_regs.hi = rsOperand;
            r3000a_regs.lo = 0xFFFFFFFF;
            
            return;
        }

        r3000a_regs.lo = rsOperand / rtOperand;
        r3000a_regs.hi = rsOperand % rtOperand;
    }

    void mfhi(reg_t rd)
    {
        r3000a_regs.gpr_regs[rd] = r3000a_regs.hi;
    }

    void mflo(reg_t rd)
    {
        r3000a_regs.gpr_regs[rd] = r3000a_regs.lo;
    }

    void mthi(reg_t rs)
    {
       r3000a_regs.hi = r3000a_regs.gpr_regs[rs];
    }

    void mtlo(reg_t rs)
    {
       r3000a_regs.lo = r3000a_regs.gpr_regs[rs];
    }

    void j(j_immed26_t dest)
    {

    }

    void jal(j_immed26_t dest)
    {

    }

    void jr(reg_t rs)
    {

    }

    void jalr(reg_t rs)
    {

    }

    void beq(reg_t rs, reg_t rt, immed16_t dest)
    {

    }

    void bne(reg_t rs, reg_t rt, immed16_t dest)
    {

    }

    void bltz(reg_t rs, immed16_t dest)
    {

    }

    void bgez(reg_t rs, immed16_t dest)
    {

    }

    void bgtz(reg_t rs, immed16_t dest)
    {

    }
    
    void blez(reg_t rs, immed16_t dest)
    {

    }

    void bltzal(reg_t rs, immed16_t dest)
    {

    }

    void bgezal(reg_t rs, immed16_t dest)
    {

    }

    void syscall(uint32_t imm20)
    {

    }

    void _break(uint32_t imm20)
    {

    }
};