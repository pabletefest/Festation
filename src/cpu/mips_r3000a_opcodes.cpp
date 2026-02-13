#include "mips_r3000a_opcodes.hpp"
#include "psx_cw33300_cpu.hpp"
#include "exceptions_handling.hpp"

#include <limits>
// #include <stdckdint.h>
#include <bit>

namespace festation
{
    static void calculateAndPerformJumpAddress(MIPS_R3000A_Core& cpu, j_immed26_t dest)
    {
        uint32_t jumpAddress = (cpu.getCPURegs().pc & 0xF0000000) | (dest << 2);

        cpu.getCPURegs().storeDelayedJump(jumpAddress);
    }

    static void calculateAndPerformBranchAddress(MIPS_R3000A_Core& cpu, immed16_t dest)
    {
        // We remove "+ 4" from the ecuation as PC already points to the address of the instructuon in the delay slot
        // In order to add 4, we should sub 4 before, so removing "+ 4" effectively leads to the same result
        int32_t branchAddress = signExtend(cpu.getCPURegs().pc) + (signExtend(dest) * 4);
        
        cpu.getCPURegs().storeDelayedJump((uint32_t)branchAddress);
    }

    void lb(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tlb\t{}, {:-X}h({})", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], signExtend(imm), g_registerNames[rs]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot() && rt != cpu.getCPURegs().getLoadReg())
            cpu.getCPURegs().consumeLoadedData();

        uint32_t cachedLoad = (uint32_t)signExtend(cpu.read8(rsValue + signExtend(imm)));
        cpu.getCPURegs().storeDelayedData(cachedLoad, rt);
    }

    void lbu(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tlbu\t{}, {:-X}h({})", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], signExtend(imm), g_registerNames[rs]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot() && rt != cpu.getCPURegs().getLoadReg())
            cpu.getCPURegs().consumeLoadedData();

        uint32_t cachedLoad = cpu.read8(rsValue + signExtend(imm));
        cpu.getCPURegs().storeDelayedData(cachedLoad, rt);
    }

    void lh(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tlh\t{}, {:-X}h({})", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], signExtend(imm), g_registerNames[rs]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot() && rt != cpu.getCPURegs().getLoadReg())
            cpu.getCPURegs().consumeLoadedData();

        uint32_t address = rsValue + signExtend(imm);

        // TODO: handle misaligned address error exceptions and invalid memory locations bus error exception
        if (handleAndSetBadVaddrReg(cpu, address, AddressBoundary::HALF_WORD_BOUNDARY))
        {
            handleException(cpu, ExcCode_AdEL);
            return;
        }

        uint32_t cachedLoad = (uint32_t)signExtend(cpu.read16(address));
        cpu.getCPURegs().storeDelayedData(cachedLoad, rt);
    }

    void lhu(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tlhu\t{}, {:-X}h({})", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], signExtend(imm), g_registerNames[rs]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot() && rt != cpu.getCPURegs().getLoadReg())
            cpu.getCPURegs().consumeLoadedData();

        uint32_t address = rsValue + signExtend(imm);

        // TODO: handle misaligned address error exceptions and invalid memory locations bus error exception
        if (handleAndSetBadVaddrReg(cpu, address, AddressBoundary::HALF_WORD_BOUNDARY))
        {
            handleException(cpu, ExcCode_AdEL);
            return;
        }

        uint32_t cachedLoad = cpu.read16(address);
        cpu.getCPURegs().storeDelayedData(cachedLoad, rt);
    }

    void lw(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tlw\t{}, {:-X}h({})", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], signExtend(imm), g_registerNames[rs]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot() && rt != cpu.getCPURegs().getLoadReg())
            cpu.getCPURegs().consumeLoadedData();

        uint32_t address = rsValue + signExtend(imm);

        // TODO: handle misaligned address error exceptions and invalid memory locations bus error exception
        if (handleAndSetBadVaddrReg(cpu, address, AddressBoundary::WORD_BOUNDARY))
        {
            handleException(cpu, ExcCode_AdEL);
            return;
        }

        uint32_t cachedLoad = cpu.read32(address);
        cpu.getCPURegs().storeDelayedData(cachedLoad, rt);
    }

    // No need to implement write-queue behaviour for store instructions

    void sb(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tsb\t{}, {:-X}h({})", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], signExtend(imm), g_registerNames[rs]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.write8(rsValue + signExtend(imm), rtValue & 0xFF);
    }

    void sh(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tsh\t{}, {:-X}h({})", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], signExtend(imm), g_registerNames[rs]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        uint32_t address = rsValue + signExtend(imm);

        if (handleAndSetBadVaddrReg(cpu, address, AddressBoundary::HALF_WORD_BOUNDARY))
        {
            handleException(cpu, ExcCode_AdES);
            return;
        }

        cpu.write16(address, rtValue & 0xFFFF);
    }

    void sw(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tsw\t{}, {:-X}h({})", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], signExtend(imm), g_registerNames[rs]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        uint32_t address = rsValue + signExtend(imm);

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (handleAndSetBadVaddrReg(cpu, address, AddressBoundary::WORD_BOUNDARY))
        {
            handleException(cpu, ExcCode_AdES);
            return;
        }

        cpu.write32(address, rtValue);
    }

    void lwr(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tlwr\t{}, {:-X}h({})", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], signExtend(imm), g_registerNames[rs]);
#endif

        constexpr size_t WORD_SIZE = sizeof(uint32_t);

        uint32_t prevRT = cpu.getCPURegs().gpr_regs[rt];
        uint32_t address = cpu.getCPURegs().gpr_regs[rs] + signExtend(imm);

        if (cpu.getCPURegs().getLoadReg() == rt)
        {
            prevRT = cpu.getCPURegs().getLoadValue();
        }
        else
        {
            cpu.getCPURegs().consumeLoadedData();
        }

        uint8_t offset = address % WORD_SIZE;
        uint32_t loadedValue = cpu.read32(address & ~offset);
        const uint8_t shiftAmount = (offset * 8);

        loadedValue &= (0xFFFFFFFFu << shiftAmount);
        loadedValue >>= shiftAmount;
        prevRT &= ~(0xFFFFFFFFu >> shiftAmount);

        cpu.getCPURegs().storeDelayedData(prevRT | loadedValue, rt);
    }

    void lwl(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tlwl\t{}, {:-X}h({})", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], signExtend(imm), g_registerNames[rs]);
#endif

        constexpr size_t WORD_SIZE = sizeof(uint32_t);
        
        uint32_t prevRT = cpu.getCPURegs().gpr_regs[rt];
        uint32_t address = cpu.getCPURegs().gpr_regs[rs] + signExtend(imm);
        
        if (cpu.getCPURegs().getLoadReg() == rt)
        {
            prevRT = cpu.getCPURegs().getLoadValue();
        }
        else
        {
            cpu.getCPURegs().consumeLoadedData();
        }

        uint8_t offset = address % WORD_SIZE;
        uint32_t loadedValue = cpu.read32(address & ~offset);
        const uint8_t shiftAmount = ((3 - offset) * 8);

        loadedValue &= (0xFFFFFFFFu >> shiftAmount);
        loadedValue <<= shiftAmount;
        prevRT &= ~(0xFFFFFFFFu << shiftAmount);

        cpu.getCPURegs().storeDelayedData(prevRT | loadedValue, rt);
    }

    void swr(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tswr\t{}, {:-X}h({})", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], signExtend(imm), g_registerNames[rs]);
#endif

        constexpr size_t WORD_SIZE = sizeof(uint32_t);

        uint32_t prevRT = cpu.getCPURegs().gpr_regs[rt];
        uint32_t address = cpu.getCPURegs().gpr_regs[rs] + signExtend(imm);

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        uint8_t offset = address % WORD_SIZE;
        uint32_t loadedValue = cpu.read32(address & ~offset);
        const uint8_t shiftAmount = (offset * 8);

        prevRT &= (0xFFFFFFFFu >> shiftAmount);
        prevRT <<= shiftAmount;
        loadedValue &= ~(0xFFFFFFFFu << shiftAmount);

        cpu.write32(address & ~offset, prevRT | loadedValue);
    }

    void swl(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tswl\t{}, {:-X}h({})", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], signExtend(imm), g_registerNames[rs]);
#endif

        constexpr size_t WORD_SIZE = sizeof(uint32_t);

        uint32_t prevRT = cpu.getCPURegs().gpr_regs[rt];
        uint32_t address = cpu.getCPURegs().gpr_regs[rs] + signExtend(imm);

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        uint8_t offset = address % WORD_SIZE;
        uint32_t loadedValue = cpu.read32(address & ~offset);
        const uint8_t shiftAmount = ((3 - offset) * 8);

        prevRT &= (0xFFFFFFFFu << shiftAmount);
        prevRT >>= shiftAmount;
        loadedValue &= ~(0xFFFFFFFFu >> shiftAmount);

        cpu.write32(address & ~offset, prevRT | loadedValue);
    }

    void add(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tadd\t{},{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rs], g_registerNames[rt]);
#endif

        int32_t result = 0;
        int32_t operand_rs =  signExtend(cpu.getCPURegs().gpr_regs[rs]);
        int32_t operand_rt =  signExtend(cpu.getCPURegs().gpr_regs[rt]);
        
        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        result = operand_rs + operand_rt;

        // #ifdef __GNUC__
        //     if (__builtin_add_overflow(r3000a_regs.gpr_regs[rs], r3000a_regs.gpr_regs[rt], &result))
        //     {
        //         // TODO: Overflow Exception
        //         return;
        //     }
        // #elifdef _MSC_VER
        //     if (_addcarry_u32(0, (int32_t)r3000a_regs.gpr_regs[rs], r3000a_regs.gpr_regs[rt], &result))
        //     {
        //         // TODO: Overflow Exception
        //         return;
        //     }
        // #endif

        if ((operand_rs ^ result) & (operand_rt ^ result) & 0x80000000) {
            handleException(cpu, ExcCode_Ov);
            return;
        } 

        cpu.getCPURegs().gpr_regs[rd] = (uint32_t)result;
    }

    void addu(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\taddu\t{},{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rs], g_registerNames[rt]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rd] = rsValue + rtValue;
    }

    void sub(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tsub\t{},{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rs], g_registerNames[rt]);
#endif

        int32_t result = 0;
        int32_t operand_rs =  signExtend(cpu.getCPURegs().gpr_regs[rs]);
        int32_t operand_rt =  signExtend(cpu.getCPURegs().gpr_regs[rt]);
        
        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        result = operand_rs - operand_rt;

        // #ifdef __GNUC__
        //     if (__builtin_sub_overflow(r3000a_regs.gpr_regs[rs], r3000a_regs.gpr_regs[rt], &result))
        //     {
        //         // TODO: Overflow Exception
        //         return;
        //     }    
        // #elifdef _MSC_VER
        //     if (_subborrow_u32(0, r3000a_regs.gpr_regs[rs], r3000a_regs.gpr_regs[rt], &result))
        //     {
        //         // TODO: Overflow Exception
        //         return;
        //     }
        // #endif

        if ((operand_rs ^ operand_rt) & (operand_rs ^ result) & 0x80000000) {
            handleException(cpu, ExcCode_Ov);
            return;
        } 

        cpu.getCPURegs().gpr_regs[rd] = (uint32_t)result;
    }

    void subu(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tsubu\t{},{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rs], g_registerNames[rt]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rd] = rsValue - rtValue;
    }
    
    void addi(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\taddi\t{},{},{:-X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], g_registerNames[rs], signExtend(imm));
#endif

        int32_t result = 0;
        int32_t operand_rs =  signExtend(cpu.getCPURegs().gpr_regs[rs]);
        int32_t operand_imm =  signExtend(imm);

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();
        
        result = operand_rs + operand_imm;

        // #ifdef __GNUC__
        //     if (__builtin_add_overflow(r3000a_regs.gpr_regs[rs], (uint32_t)(uint16_t)imm, &result))
        //     {
        //         // TODO: Overflow Exception
        //         return;
        //     }
        // #elifdef _MSC_VER
        //     if (_addcarry_u32(0, r3000a_regs.gpr_regs[rs], r3000a_regs.gpr_regs[rt], &result))
        //     {
        //         // TODO: Overflow Exception
        //         return;
        //     }
        // #endif

        if ((operand_rs ^ result) & (operand_imm ^ result) & 0x80000000) {
            handleException(cpu, ExcCode_Ov);
            return;
        } 

        cpu.getCPURegs().gpr_regs[rt] = (uint32_t)result;
    }

    void addiu(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\taddiu\t{},{},{:-X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], g_registerNames[rs], signExtend(imm));
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rt] = rsValue + signExtend(imm);
    }

    void slt(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tslt\t{},{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rs], g_registerNames[rt]);
#endif
        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (signExtend(rsValue) < signExtend(rtValue))
            cpu.getCPURegs().gpr_regs[rd] = 1;
        else
            cpu.getCPURegs().gpr_regs[rd] = 0;
    }

    void sltu(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tsltu\t{},{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rs], g_registerNames[rt]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (rsValue < rtValue)
            cpu.getCPURegs().gpr_regs[rd] = 1;
        else
            cpu.getCPURegs().gpr_regs[rd] = 0;
    }

    void slti(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tslti\t{},{},{:-X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], g_registerNames[rs], signExtend(imm));
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (signExtend(rsValue) < signExtend(imm))
            cpu.getCPURegs().gpr_regs[rt] = 1;
        else
            cpu.getCPURegs().gpr_regs[rt] = 0;
    }

    void sltiu(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tsltiu\t{},{},{:-X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], g_registerNames[rs], signExtend(imm));
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (rsValue < (uint32_t)signExtend(imm))
            cpu.getCPURegs().gpr_regs[rt] = 1;
        else
            cpu.getCPURegs().gpr_regs[rt] = 0;
    }

    void _and(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tand\t{},{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rs], g_registerNames[rt]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rd] = rsValue & rtValue;
    }

    void _or(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tor\t{},{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rs], g_registerNames[rt]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rd] = rsValue | rtValue;
    }

    void _xor(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\txor\t{},{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rs], g_registerNames[rt]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rd] = rsValue ^ rtValue;
    }

    void nor(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rs, reg_t rt)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tnor\t{},{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rs], g_registerNames[rt]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rd] = 0xFFFFFFFF ^ (rsValue | rtValue);
    }

    void andi(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tandi\t{},{},{:-X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], g_registerNames[rs], signExtend(imm));
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rt] = rsValue & imm;
    }

    void ori(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tori\t{},{},{:-X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], g_registerNames[rs], signExtend(imm));
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rt] = rsValue | imm;
    }

    void xori(MIPS_R3000A_Core& cpu, reg_t rt, reg_t rs, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\txori\t{},{},{:-X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], g_registerNames[rs], signExtend(imm));
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rt] = rsValue ^ imm;
    }

    void sllv(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rt, reg_t rs)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tsllv\t{},{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rs], g_registerNames[rt]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rd] = rtValue << (rsValue & 0x1F);
    }

    void srlv(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rt, reg_t rs)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tsrlv\t{},{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rs], g_registerNames[rt]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rd] = rtValue >> (rsValue & 0x1F);
    }

    void srav(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rt, reg_t rs)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tsrav\t{},{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rs], g_registerNames[rt]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rd] = signExtend(rtValue) >> (int8_t)(rsValue  & 0x1F);
    }

    void sll(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rt, shift_t imm)
    {
#ifdef DISASSEMBLY
        if (rd == 0 && rt == 0 && imm == 0)
        {
            LOG_DEBUG("0x{:08X} {:08X}\tnop", \
                cpu.getCPURegs().currentPC, cpu.getCurrentInstruction());
        }
        else
        {
            LOG_DEBUG("0x{:08X} {:08X}\tsll\t{},{},{:-X}h", \
                cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rt], signExtend(imm));
        }
#endif

        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rd] = rtValue << imm;
    }

    void srl(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rt, shift_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tsrl\t{},{},{:-X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rt], signExtend(imm));
#endif

        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rd] = rtValue >> imm;
    }

    void sra(MIPS_R3000A_Core& cpu, reg_t rd, reg_t rt, shift_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tsra\t{},{},{:-X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd], g_registerNames[rt], signExtend(imm));
#endif

        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rd] = signExtend(rtValue) >> (int8_t)imm;
    }

    void lui(MIPS_R3000A_Core& cpu, reg_t rt, immed16_t imm)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tlui\t{},{:-X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rt], imm);
#endif

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rt] = imm << 16;
    }

    void mult(MIPS_R3000A_Core& cpu, reg_t rs, reg_t rt)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tmult\t{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs], g_registerNames[rt]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        int64_t result = (int64_t)signExtend(rsValue) * (int64_t)signExtend(rtValue);

        cpu.getCPURegs().hi = (uint64_t)(result >> 32) & 0xFFFFFFFF;
        cpu.getCPURegs().lo = (uint64_t)result & 0xFFFFFFFF;
    }

    void multu(MIPS_R3000A_Core& cpu, reg_t rs, reg_t rt)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tmultu\t{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs], g_registerNames[rt]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        uint64_t result = (uint64_t)rsValue * (uint64_t)rtValue;

        cpu.getCPURegs().hi = (result >> 32) & 0xFFFFFFFF;
        cpu.getCPURegs().lo = result & 0xFFFFFFFF;
    }

    void div(MIPS_R3000A_Core& cpu, reg_t rs, reg_t rt)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tdiv\t{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs], g_registerNames[rt]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        int32_t rsOperand = signExtend(rsValue);
        int32_t rtOperand = signExtend(rtValue);

        if (rtOperand == 0)
        {
            cpu.getCPURegs().hi = rsOperand;

            if (rsOperand >= 0)
                cpu.getCPURegs().lo = -1;
            else
                cpu.getCPURegs().lo = 1;

            return;
        }
        else if (rtOperand == -1 && rsOperand == std::numeric_limits<std::int32_t>::min())
        {
            cpu.getCPURegs().hi = 0;
            cpu.getCPURegs().lo = rsOperand; // std::numeric_limits<std::int32_t>::min() or (int32_t)0x80000000 or -0x80000000
            
            return;
        }

        cpu.getCPURegs().lo = (uint32_t)(rsOperand / rtOperand);
        cpu.getCPURegs().hi = (uint32_t)(rsOperand % rtOperand);
    }

    void divu(MIPS_R3000A_Core& cpu, reg_t rs, reg_t rt)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tdivu\t{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs], g_registerNames[rt]);
#endif

        uint32_t rsOperand = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtOperand = cpu.getCPURegs().gpr_regs[rt];
        
        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (rtOperand == 0)
        {
            cpu.getCPURegs().hi = rsOperand;
            cpu.getCPURegs().lo = 0xFFFFFFFF;
            
            return;
        }

        cpu.getCPURegs().lo = rsOperand / rtOperand;
        cpu.getCPURegs().hi = rsOperand % rtOperand;
    }

    void mfhi(MIPS_R3000A_Core& cpu, reg_t rd)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tmfhi\t{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd]);
#endif

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rd] = cpu.getCPURegs().hi;
    }

    void mflo(MIPS_R3000A_Core& cpu, reg_t rd)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tmflo\t{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rd]);
#endif

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().gpr_regs[rd] = cpu.getCPURegs().lo;
    }

    void mthi(MIPS_R3000A_Core& cpu, reg_t rs)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tmthi\t{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().hi = rsValue;
    }

    void mtlo(MIPS_R3000A_Core& cpu, reg_t rs)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tmtlo\t{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs]);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        cpu.getCPURegs().lo = rsValue;
    }

    void j(MIPS_R3000A_Core& cpu, j_immed26_t dest)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tj\t{:X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), dest);
#endif

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        calculateAndPerformJumpAddress(cpu, dest);
    }

    void jal(MIPS_R3000A_Core& cpu, j_immed26_t dest)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tjal\t{:X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), dest);
#endif

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        // Skip branch delay slot instruction so it's 8 bytes next instruction when returning
        // We only add "+ 4" because PC was already incremented by 4 after fetching the instruction
        cpu.getCPURegs().gpr_regs[ra] = cpu.getCPURegs().currentPC + 8; // Effectively instruction address + 8
        calculateAndPerformJumpAddress(cpu, dest);
    }

    void jr(MIPS_R3000A_Core& cpu, reg_t rs)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tjr\t{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs]);
#endif

        const uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        // TODO: arise address error (AdEL) exception if jumping to unaligned address
        cpu.getCPURegs().storeDelayedJump(rsValue);
    }

    void jalr(MIPS_R3000A_Core& cpu, reg_t rs, reg_t rd)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tjalr\t{},{}", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs], g_registerNames[rd]);
#endif

        const uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (rd == 0) // rd omitted in the assembly instruction
        {
            cpu.getCPURegs().gpr_regs[ra] = cpu.getCPURegs().currentPC + 8;
        }
        else
        {
            cpu.getCPURegs().gpr_regs[rd] = cpu.getCPURegs().currentPC + 8;
        }

        // TODO: arise address error (AdEL) exception if jumping to unaligned address
        cpu.getCPURegs().storeDelayedJump(rsValue);
    }

    void beq(MIPS_R3000A_Core& cpu, reg_t rs, reg_t rt, immed16_t dest)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tbeq\t{},{},{:X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs], g_registerNames[rt], dest);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (rsValue == rtValue)
        {
            calculateAndPerformBranchAddress(cpu, dest);
        }
    }

    void bne(MIPS_R3000A_Core& cpu, reg_t rs, reg_t rt, immed16_t dest)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tbne\t{},{},{:X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs], g_registerNames[rt], dest);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];
        uint32_t rtValue = cpu.getCPURegs().gpr_regs[rt];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (rsValue != rtValue)
        {
            calculateAndPerformBranchAddress(cpu, dest);
        }
    }

    void bltz(MIPS_R3000A_Core& cpu, reg_t rs, immed16_t dest)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tbltz\t{},{:X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs], dest);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (signExtend(rsValue) < 0)
        {
            calculateAndPerformBranchAddress(cpu, dest);
        }
    }

    void bgez(MIPS_R3000A_Core& cpu, reg_t rs, immed16_t dest)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tbgez\t{},{:X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs], dest);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (signExtend(rsValue) >= 0)
        {
            calculateAndPerformBranchAddress(cpu, dest);
        }
    }

    void bgtz(MIPS_R3000A_Core& cpu, reg_t rs, immed16_t dest)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tbgtz\t{},{:X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs], dest);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (signExtend(rsValue) > 0)
        {
            calculateAndPerformBranchAddress(cpu, dest);
        }
    }
    
    void blez(MIPS_R3000A_Core& cpu, reg_t rs, immed16_t dest)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tblez\t{},{:X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs], dest);
#endif

        uint32_t rsValue = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (signExtend(rsValue) <= 0)
        {
            calculateAndPerformBranchAddress(cpu, dest);
        }
    }

    void bltzal(MIPS_R3000A_Core& cpu, reg_t rs, immed16_t dest)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tbltzal\t{},{:X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs], dest);
#endif

        uint32_t cmpReg = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (rs == ra)
            cmpReg = cpu.getCPURegs().gpr_regs[ra]; // We compare against prev $ra reg before linking and modify its value

        cpu.getCPURegs().gpr_regs[ra] = cpu.getCPURegs().currentPC + 8;

        if (signExtend(cmpReg) < 0)
        {
            calculateAndPerformBranchAddress(cpu, dest);
        }
    }

    void bgezal(MIPS_R3000A_Core& cpu, reg_t rs, immed16_t dest)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tbgezal\t{},{:X}h", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction(), g_registerNames[rs], dest);
#endif

        uint32_t cmpReg = cpu.getCPURegs().gpr_regs[rs];

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        if (rs == ra)
            cmpReg = cpu.getCPURegs().gpr_regs[ra]; // We compare against prev $ra reg before linking and modify its value

        cpu.getCPURegs().gpr_regs[ra] = cpu.getCPURegs().currentPC + 8;

        if (signExtend(cmpReg) >= 0)
        {
            calculateAndPerformBranchAddress(cpu,  dest);
        }
    }

    void syscall(MIPS_R3000A_Core& cpu, uint32_t imm20)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tsyscall", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction());
#endif

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        handleException(cpu, ExcCode_Syscall);
    }

    void _break(MIPS_R3000A_Core& cpu, uint32_t imm20)
    {
#ifdef DISASSEMBLY
        LOG_DEBUG("0x{:08X} {:08X}\tbeq", \
            cpu.getCPURegs().currentPC, cpu.getCurrentInstruction());
#endif

        if (cpu.getCPURegs().isLoadDelaySlot())
            cpu.getCPURegs().consumeLoadedData();

        handleException(cpu, ExcCode_BP);
    }
};