#include "mips_r3000a_opcodes.hpp"
#include "psx_system.hpp"
#include "psx_cpu_state.hpp"

#include <limits>

namespace festation
{
    extern PSXSystem psxSystem;
    extern PSXRegs r3000a_regs;

    static void calculateAndPerformBranchAddress(immed16_t dest)
    {
        int32_t branchAddr = (int32_t)r3000a_regs.pc + 4 + ((int16_t)dest * 4);
        // r3000a_regs.pc = (uint32_t)branchAddr;
        r3000a_regs.storeDelayedJump((uint32_t)branchAddr);
    }

    void lb(reg_t rt, reg_t rs, immed16_t imm)
    {
        // TODO: handle misaligned address error exceptions and invalid memory locations bus error exception

        // r3000a_regs.gpr_regs[rt] = (uint32_t)(int32_t)(int8_t)psxSystem.read8(r3000a_regs.gpr_regs[rs] + imm);
        uint32_t cachedLoad = (uint32_t)(int32_t)(int8_t)psxSystem.read8(r3000a_regs.gpr_regs[rs] + imm);
        r3000a_regs.storeDelayedData(cachedLoad, rt);
    }

    void lbu(reg_t rt, reg_t rs, immed16_t imm)
    {
        // TODO: handle misaligned address error exceptions and invalid memory locations bus error exception     

        // r3000a_regs.gpr_regs[rt] = psxSystem.read8(r3000a_regs.gpr_regs[rs] + imm);
        uint32_t cachedLoad = psxSystem.read8(r3000a_regs.gpr_regs[rs] + imm);
        r3000a_regs.storeDelayedData(cachedLoad, rt);
    }

    void lh(reg_t rt, reg_t rs, immed16_t imm)
    {
        // TODO: handle misaligned address error exceptions and invalid memory locations bus error exception

        // r3000a_regs.gpr_regs[rt] = (uint32_t)(int32_t)(int16_t)psxSystem.read16(r3000a_regs.gpr_regs[rs] + imm);
        uint32_t cachedLoad = (uint32_t)(int32_t)(int16_t)psxSystem.read16(r3000a_regs.gpr_regs[rs] + imm);
        r3000a_regs.storeDelayedData(cachedLoad, rt);
    }

    void lhu(reg_t rt, reg_t rs, immed16_t imm)
    {
        // TODO: handle misaligned address error exceptions and invalid memory locations bus error exception

        // r3000a_regs.gpr_regs[rt] = psxSystem.read16(r3000a_regs.gpr_regs[rs] + imm);
        uint32_t cachedLoad = psxSystem.read16(r3000a_regs.gpr_regs[rs] + imm);
        r3000a_regs.storeDelayedData(cachedLoad, rt);
    }

    void lw(reg_t rt, reg_t rs, immed16_t imm)
    {
        // TODO: handle invalid memory locations bus error exception

        // r3000a_regs.gpr_regs[rt] = psxSystem.read32(r3000a_regs.gpr_regs[rs] + imm);
        uint32_t cachedLoad = psxSystem.read32(r3000a_regs.gpr_regs[rs] + imm);
        r3000a_regs.storeDelayedData(cachedLoad, rt);
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
        r3000a_regs.pc = (r3000a_regs.pc & 0xF0000000) | (dest << 2); // High 4 bits of PC + 26-bit address * 4 (4 bytes multiples so lower 2 bits not addressable)
    }

    void jal(j_immed26_t dest)
    {
        r3000a_regs.gpr_regs[ra] = r3000a_regs.pc + 8; // Skip branch delay slot instruction so it's 8 bytes next instruction when returning
        r3000a_regs.pc = (r3000a_regs.pc & 0xF0000000) | (dest << 2);
    }

    void jr(reg_t rs)
    {
        // TODO: arise address error (AdEL) exception if jumping to unaligned address
        r3000a_regs.pc = r3000a_regs.gpr_regs[rs];
    }

    void jalr(reg_t rs, reg_t rd)
    {
        if (rd == 0) // rd omitted in the assembly instruction
        {
            r3000a_regs.gpr_regs[ra] = r3000a_regs.pc + 8;
        }
        else
        {
            r3000a_regs.gpr_regs[rd] = r3000a_regs.pc + 8;
        }

        // TODO: arise address error (AdEL) exception if jumping to unaligned address
        r3000a_regs.pc = r3000a_regs.gpr_regs[rs];
    }

    void beq(reg_t rs, reg_t rt, immed16_t dest)
    {
        if (r3000a_regs.gpr_regs[rs] == r3000a_regs.gpr_regs[rt])
        {
            calculateAndPerformBranchAddress(dest);
        }
    }

    void bne(reg_t rs, reg_t rt, immed16_t dest)
    {
        if (r3000a_regs.gpr_regs[rs] != r3000a_regs.gpr_regs[rt])
        {
            calculateAndPerformBranchAddress(dest);
        }
    }

    void bltz(reg_t rs, immed16_t dest)
    {
        if (((int32_t)r3000a_regs.gpr_regs[rs]) < 0)
        {
            calculateAndPerformBranchAddress(dest);
        }
    }

    void bgez(reg_t rs, immed16_t dest)
    {
        if (((int32_t)r3000a_regs.gpr_regs[rs]) >= 0)
        {
            calculateAndPerformBranchAddress(dest);
        }
    }

    void bgtz(reg_t rs, immed16_t dest)
    {
        if (((int32_t)r3000a_regs.gpr_regs[rs]) > 0)
        {
            calculateAndPerformBranchAddress(dest);
        }
    }
    
    void blez(reg_t rs, immed16_t dest)
    {
        if (((int32_t)r3000a_regs.gpr_regs[rs]) <= 0)
        {
            calculateAndPerformBranchAddress(dest);
        }
    }

    void bltzal(reg_t rs, immed16_t dest)
    {
        uint32_t cmpReg = r3000a_regs.gpr_regs[rs];

        if (rs == ra)
            cmpReg = r3000a_regs.gpr_regs[ra]; // We compare against prev $ra reg before linking and modify its value

        r3000a_regs.gpr_regs[ra] = r3000a_regs.pc + 8; 

        if (((int32_t)cmpReg) < 0)
        {
            calculateAndPerformBranchAddress(dest);
        }
    }

    void bgezal(reg_t rs, immed16_t dest)
    {
        uint32_t cmpReg = r3000a_regs.gpr_regs[rs];

        if (rs == ra)
            cmpReg = r3000a_regs.gpr_regs[ra]; // We compare against prev $ra reg before linking and modify its value

        r3000a_regs.gpr_regs[ra] = r3000a_regs.pc + 8; 

        if (((int32_t)cmpReg) >= 0)
        {
            calculateAndPerformBranchAddress(dest);
        }
    }

    void syscall(uint32_t imm20)
    {

    }

    void _break(uint32_t imm20)
    {

    }
};