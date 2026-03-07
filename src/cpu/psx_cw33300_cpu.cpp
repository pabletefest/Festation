#include "psx_cw33300_cpu.hpp"
#include "psx_system.hpp"
#include "mips_r3000a_opcodes.hpp"
#include "coprocessor_cp0_opcodes.hpp"
#include "exceptions_handling.hpp"
#include "utils/logger.hpp"
#include "memory/memory_map_masks.hpp"

#include <cstring>
#include <cassert>

namespace festation
{
    static constexpr uint32_t INSTRUCTION_SIZE = 4;
    static constexpr uintptr_t RESET_VECTOR = 0xBCF00000;
    static constexpr uint8_t INSTRUCTION_CYCLES_AVERAGE = 2;
};

festation::MIPS_R3000A_Core::MIPS_R3000A_Core(PSXSystem* device)
    : system(device)
{
    std::memset((void*) &r3000a_regs, 0, sizeof(PSXRegs));

    reset();
}

void festation::MIPS_R3000A_Core::reset()
{
    handleReset(*this);
}

uint8_t festation::MIPS_R3000A_Core::read8(uint32_t address)
{
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;
    
    if (masked_address >= SCRATCHPAD_START && masked_address <= SCRATCHPAD_END)
    {
        return scratchpadCache[masked_address & SCRATCHPAD_SIZE_MASK];
    }

    return system->read8(address);
}

uint16_t festation::MIPS_R3000A_Core::read16(uint32_t address)
{
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address >= SCRATCHPAD_START && masked_address <= SCRATCHPAD_END)
    {
        return scratchpadCache[masked_address & SCRATCHPAD_SIZE_MASK] |
            (scratchpadCache[(masked_address + 1) & SCRATCHPAD_SIZE_MASK] << 8);
    }

    return system->read16(address);
}

uint32_t festation::MIPS_R3000A_Core::read32(uint32_t address)
{
    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address >= SCRATCHPAD_START && masked_address <= SCRATCHPAD_END)
    {
        return scratchpadCache[masked_address & SCRATCHPAD_SIZE_MASK] |
            (scratchpadCache[(masked_address + 1) & SCRATCHPAD_SIZE_MASK] << 8) |
            (scratchpadCache[(masked_address + 2) & SCRATCHPAD_SIZE_MASK] << 16) |
            (scratchpadCache[(masked_address + 3) & SCRATCHPAD_SIZE_MASK] << 24);
    }

    uint32_t value = system->read32(address);

    /*if (value == 0x801ff014) {
        LOG_DEBUG("Reading {:08X}h from 0x{:08X}", value, address);
        printCPUState();
        printCOP0State();
    }*/

    return value;
}

void festation::MIPS_R3000A_Core::write8(uint32_t address, uint8_t value)
{
    if (isCacheIsolated())
        return;

    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address >= SCRATCHPAD_START && masked_address <= SCRATCHPAD_END)
    {
        scratchpadCache[masked_address & SCRATCHPAD_SIZE_MASK] = value;
        return;
    }

    system->write8(address, value);
}

void festation::MIPS_R3000A_Core::write16(uint32_t address, uint16_t value)
{
    if (isCacheIsolated())
        return;

    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address >= SCRATCHPAD_START && masked_address <= SCRATCHPAD_END)
    {
        scratchpadCache[masked_address & SCRATCHPAD_SIZE_MASK] = value & 0xFF;
        scratchpadCache[(masked_address + 1) & SCRATCHPAD_SIZE_MASK] = value >> 8;
        return;
    }

    system->write16(address, value);
}

void festation::MIPS_R3000A_Core::write32(uint32_t address, uint32_t value)
{
    if (isCacheIsolated())
        return;

    uint32_t masked_address = address & PHYSICAL_MEMORY_MASK;

    if (masked_address >= SCRATCHPAD_START && masked_address <= SCRATCHPAD_END)
    {
        scratchpadCache[masked_address & SCRATCHPAD_SIZE_MASK] = value & 0xFF;
        scratchpadCache[(masked_address + 1) & SCRATCHPAD_SIZE_MASK] = (value >> 8) & 0xFF;
        scratchpadCache[(masked_address + 2) & SCRATCHPAD_SIZE_MASK] = (value >> 16) & 0xFF;
        scratchpadCache[(masked_address + 3) & SCRATCHPAD_SIZE_MASK] = (value >> 24) & 0xFF;
        return;
    }

    system->write32(address, value);

    /*if (value == 0x801ff014) {
        LOG_DEBUG("Writting {:08X}h to 0x{:08X}", value, address);
        printCPUState();
        printCOP0State();
    }*/
}

uint8_t festation::MIPS_R3000A_Core::executeInstruction()
{
    const bool isBranchDelayPending = r3000a_regs.isBranchDelaySlot();

    if (r3000a_regs.currentPC & 3)
    {
        handleException(*this, ExcCode_AdEL);
    }

    currentInstruction = fetchInstruction();

    //LOG_DEBUG("* Executing instruction: 0x{:08X}  at address 0x{:08X} *", instruction, r3000a_regs.pc - 4);

    MipsInstruction instructionCodePtr = decodeInstruction(currentInstruction);

    instructionCodePtr();

    r3000a_regs.gpr_regs[0] = 0; // $0 or $zero is always zero

    if (isBranchDelayPending)
        r3000a_regs.performDelayedJump();

    return INSTRUCTION_CYCLES_AVERAGE;
}

void festation::MIPS_R3000A_Core::clockCycles(uint32_t cycles)
{
}

festation::PSXRegs& festation::MIPS_R3000A_Core::getCPURegs()
{
    return r3000a_regs;
}

festation::COP0SystemControlRegs& festation::MIPS_R3000A_Core::getCOP0Regs()
{
    return cop0_state;
}

bool festation::MIPS_R3000A_Core::isCacheIsolated() const
{
    return (cop0_state.getCop0RegisterValue(SR) & CACHE_ISOLATION_BIT_MASK) != 0;
}

void festation::MIPS_R3000A_Core::printCPUState()
{
    const PSXRegs& regs = this->getCPURegs();
    LOG_DEBUG("R0: {:08X}h - R1: {:08X}h - R2: {:08X}h - R3: {:08X}h - R4: {:08X}h - R5: {:08X}h - R6: {:08X}h R7: {:08X}h",
        regs.gpr_regs[0], regs.gpr_regs[1], regs.gpr_regs[2], regs.gpr_regs[3], regs.gpr_regs[4], regs.gpr_regs[5], regs.gpr_regs[6], regs.gpr_regs[7]);
    LOG_DEBUG("R8: {:08X}h - R9: {:08X}h - R10: {:08X}h - R11: {:08X}h - R12: {:08X}h - R13: {:08X}h - R114: {:08X}h R15: {:08X}h",
        regs.gpr_regs[8], regs.gpr_regs[9], regs.gpr_regs[10], regs.gpr_regs[11], regs.gpr_regs[12], regs.gpr_regs[13], regs.gpr_regs[14], regs.gpr_regs[15]);
    LOG_DEBUG("R16: {:08X}h - R17: {:08X}h - R18: {:08X}h - R19: {:08X}h - R20: {:08X}h - R21: {:08X}h - R22: {:08X}h R23: {:08X}h",
        regs.gpr_regs[16], regs.gpr_regs[17], regs.gpr_regs[18], regs.gpr_regs[19], regs.gpr_regs[20], regs.gpr_regs[21], regs.gpr_regs[22], regs.gpr_regs[23]);
    LOG_DEBUG("R24: {:08X}h - R25: {:08X}h - R26: {:08X}h - R27: {:08X}h - R28: {:08X}h - R29: {:08X}h - R30: {:08X}h R31: {:08X}h",
        regs.gpr_regs[24], regs.gpr_regs[25], regs.gpr_regs[26], regs.gpr_regs[27], regs.gpr_regs[28], regs.gpr_regs[29], regs.gpr_regs[30], regs.gpr_regs[31]);

    LOG_DEBUG("PC (ahead): {:08X}h - Current PC: {:08X}h - HI: {:08X}h - LO: {:08X}h",
        regs.pc, regs.currentPC, regs.hi, regs.lo);

    LOG_DEBUG("");
    LOG_DEBUG("-----------------------------------------------------------------------------------");
    LOG_DEBUG("");
}

void festation::MIPS_R3000A_Core::printCOP0State()
{
    const COP0SystemControlRegs& cop0Regs = this->getCOP0Regs();

    LOG_DEBUG("BadVaddr: {:08X}h - SR: {:08X}h - CAUSE: {:08X}h - EPC: {:08X}h",
        cop0Regs.BadVaddr, cop0Regs.SR.r, cop0Regs.CAUSE.r, cop0Regs.EPC);

    LOG_DEBUG("");
    LOG_DEBUG("-----------------------------------------------------------------------------------");
    LOG_DEBUG("");
}


uint32_t festation::MIPS_R3000A_Core::fetchInstruction()
{
    uint32_t instruction = read32(r3000a_regs.pc);

    r3000a_regs.currentPC = r3000a_regs.pc;

    r3000a_regs.pc += INSTRUCTION_SIZE;

    return instruction;
}

festation::MipsInstruction festation::MIPS_R3000A_Core::decodeInstruction(uint32_t instruction)
{
    uint8_t opcode = getInstOpcode(instruction);


    switch(opcode)
    {
    case 0x0:   // R-FORMAT INSTRUCTION or SPECIAL
        {
            uint8_t function = getInstFunctionOperation(instruction);
            reg_t rd = getInstDestRegEncoding<EncodingType::REGISTER>(instruction);
            reg_t rt = getInstSrcRegEncoding<EncodingType::REGISTER, SrcRegs::RT>(instruction);
            reg_t rs = getInstSrcRegEncoding<EncodingType::REGISTER, SrcRegs::RS>(instruction);
            shift_t shift = getInstShiftAmount(instruction);

            switch(function)
            {
            case 0x00:
                return [=]() { sll(*this, rd, rt, shift); };
            case 0x02:
                return [=]() { srl(*this, rd, rt, shift); };
            case 0x03:
                return [=]() { sra(*this, rd, rt, shift); };
            case 0x04:
                return [=]() { sllv(*this, rd, rt, rs); };
            case 0x06:
                return [=]() { srlv(*this, rd, rt, rs); };
            case 0x07:
                return [=]() { srav(*this, rd, rt, rs); };
            case 0x08:
                return [=]() { jr(*this, rs); };
            case 0x09:
                return [=]() { jalr(*this, rs, rd); };
            case 0x0C:
                return [=]() { syscall(*this, getSyscallBreakCode(instruction)); };
            case 0x0D:
                return [=]() { _break(*this, getSyscallBreakCode(instruction)); };
            case 0x10:
                return [=]() { mfhi(*this, rd); };
            case 0x11:
                return [=]() { mthi(*this, rs); };
            case 0x12:
                return [=]() { mflo(*this, rd); };
            case 0x13:
                return [=]() { mtlo(*this, rs); };
            case 0x18:
                return [=]() { mult(*this, rs, rt); };
            case 0x19:
                return [=]() { multu(*this, rs, rt); };
            case 0x1A:
                return [=]() { div(*this, rs, rt); };
            case 0x1B:
                return [=]() { divu(*this, rs, rt); };
            case 0x20:
                return [=]() { add(*this, rd, rs, rt); };
            case 0x21:
                return [=]() { addu(*this, rd, rs, rt); };
            case 0x22:
                return [=]() { sub(*this, rd, rs, rt); };
            case 0x23:
                return [=]() { subu(*this, rd, rs, rt); };
            case 0x24:
                return [=]() { _and(*this, rd, rs, rt); };
            case 0x25:
                return [=]() { _or(*this, rd, rs, rt); };
            case 0x26:
                return [=]() { _xor(*this, rd, rs, rt); };
            case 0x27:
                return [=]() { nor(*this, rd, rs, rt); };
            case 0x2A:
                return [=]() { slt(*this, rd, rs, rt); };
            case 0x2B:
                return [=]() { sltu(*this, rd, rs, rt); };
            default:
                LOG_ERROR("Unimplemented or invalid R-type instruction! Function opcode: {:02X} - from hex MIPS instruction encoding ({:08X})\n", function, instruction);
                return []() {};
            }
        }
        break;
    case 0x2:   // J-FORMAT INSTRUCTION "J"
        return [=]() { j(*this, getInstAddress(instruction)); };
    case 0x3:   // J-FORMAT INSTRUCTION "JAL"
        return [=]() { jal(*this, getInstAddress(instruction)); };
    default:    // I-FORMAT INSTRUCTION
        {
            uint8_t opcode = getInstOpcode(instruction);
            reg_t rt = getInstDestRegEncoding<EncodingType::IMMEDIATE>(instruction);
            reg_t rs = getInstSrcRegEncoding<EncodingType::IMMEDIATE, SrcRegs::RS>(instruction);
            immed16_t imm16 = getInstImmediate(instruction);

            switch (opcode)
            {
            case 0x01:
            {
                // To be able to know which BcondZ instruction is, we look the encoding in rt reg field bits
                bool ezBit = (rt & 0x01) == 0x01;
                bool linkBit = ((rt >> 1) & 0x0F) == 0x08;

                if (!ezBit && !linkBit)
                {
                    return [=]() { bltz(*this, rs, imm16); };
                }
                else if (ezBit && !linkBit)
                {
                    return [=]() { bgez(*this, rs, imm16); };  
                }
                else if (!ezBit && linkBit)
                {
                    return [=]() { bltzal(*this, rs, imm16); };
                }
                else if (ezBit && linkBit)
                {
                    return [=]() { bgezal(*this, rs, imm16); };
                }
                else
                {
                    LOG_ERROR("Unimplemented or invalid I-type BcondZ instruction! rt bits: {:02} - from hex MIPS instruction encoding ({:08X})\n", rt, instruction);
                    return []() {};
                }
            }
            case 0x04:
                return [=]() { beq(*this, rs, rt, imm16); };
            case 0x05:
                return [=]() { bne(*this, rs, rt, imm16); };
            case 0x06:
                return [=]() { blez(*this, rs, imm16); };
            case 0x07:
                return [=]() { bgtz(*this, rs, imm16); };
            case 0x08:
                return [=]() { addi(*this, rt, rs, imm16); };
            case 0x09:
                return [=]() { addiu(*this, rt, rs, imm16); };
            case 0x0A:
                return [=]() { slti(*this, rt, rs, imm16); };
            case 0x0B:
                return [=]() { sltiu(*this, rt, rs, imm16); };
            case 0x0C:
                return [=]() { andi(*this, rt, rs, imm16); };
            case 0x0D:
                return [=]() { ori(*this, rt, rs, imm16); };
            case 0x0E:
                return [=]() { xori(*this, rt, rs, imm16); };
            case 0x0F:
                return [=]() { lui(*this, rt, imm16); };
            case 0x10: // COP0
                if (rs == 0b10000)
                {
                    // We don't check last 6 bits because PS1 CPU doesn't have TLB
                    return [=]() { rfe(*this); };
                }
                else
                {
                    reg_t rtcop0 = getInstSrcRegEncoding<EncodingType::REGISTER, SrcRegs::RT>(instruction);
                    reg_t rdcop0 = getInstDestRegEncoding<EncodingType::REGISTER>(instruction);

                    switch (rs) // We don't need to check for more opcodes on COP0
                    {
                    case 0b00000:
                        return [=]() { mfc0(*this, rtcop0, rdcop0); };
                    case 0b00100:
                        return [=]() { mtc0(*this, rtcop0, rdcop0); };
                    default:
                        LOG_ERROR("Unimplemented or invalid COP0 instruction! Instruction opcode: {:02} - from hex MIPS instruction encoding ({:08X})\n", opcode, instruction);
                        return []() {};
                    }
                }
            case 0x11: // COP1
                LOG_ERROR("Unimplemented or invalid COP1 instruction! Instruction opcode: {:02} - from hex MIPS instruction encoding ({:08X})\n", opcode, instruction);
                return []() {};
            case 0x12: // COP2
                LOG_ERROR("Unimplemented or invalid COP2 instruction! Instruction opcode: {:02} - from hex MIPS instruction encoding ({:08X})\n", opcode, instruction);
                return []() {};
            case 0x13: // COP3
                LOG_ERROR("Unimplemented or invalid COP3 instruction! Instruction opcode: {:02} - from hex MIPS instruction encoding ({:08X})\n", opcode, instruction);
                return []() {};
            case 0x20:
                return [=]() { lb(*this, rt, rs, imm16); };
            case 0x21:
                return [=]() { lh(*this, rt, rs, imm16); };
            case 0x22:
                return [=]() { lwl(*this, rt, rs, imm16); };
            case 0x23:
                return [=]() { lw(*this, rt, rs, imm16); };
            case 0x24:
                return [=]() { lbu(*this, rt, rs, imm16); };
            case 0x25:
                return [=]() { lhu(*this, rt, rs, imm16); };
            case 0x26:
                return [=]() { lwr(*this, rt, rs, imm16); };
            case 0x28:
                return [=]() { sb(*this, rt, rs, imm16); };
            case 0x29:
                return [=]() { sh(*this, rt, rs, imm16); };
            case 0x2A:
                return [=]() { swl(*this, rt, rs, imm16); };
            case 0x2B:
                return [=]() { sw(*this, rt, rs, imm16); };
            case 0x2E:
                return [=]() { swr(*this, rt, rs, imm16); };
            case 0x30: // COP0
                return [=]() { lwc0(*this, rt, rs, imm16); };
            case 0x31: // COP1
                LOG_ERROR("Unimplemented or invalid COP1 instruction! Instruction opcode: {:02} - from hex MIPS instruction encoding ({:08X})\n", opcode, instruction);
                return []() {};
            case 0x32: // COP2
                LOG_ERROR("Unimplemented or invalid COP2 instruction! Instruction opcode: {:02} - from hex MIPS instruction encoding ({:08X})\n", opcode, instruction);
                return []() {};
            case 0x33: // COP3
                LOG_ERROR("Unimplemented or invalid COP3 instruction! Instruction opcode: {:02} - from hex MIPS instruction encoding ({:08X})\n", opcode, instruction);
                return []() {};
            case 0x38: // COP0
                return [=]() { swc0(*this, rt, rs, imm16); };
            case 0x39: // COP1
                LOG_ERROR("Unimplemented or invalid COP1 instruction! Instruction opcode: {:02} - from hex MIPS instruction encoding ({:08X})\n", opcode, instruction);
                return []() {};
            case 0x3A: // COP2
                LOG_ERROR("Unimplemented or invalid COP2 instruction! Instruction opcode: {:02} - from hex MIPS instruction encoding ({:08X})\n", opcode, instruction);
                return []() {};
            case 0x3B: // COP3
                LOG_ERROR("Unimplemented or invalid COP3 instruction! Instruction opcode: {:02} - from hex MIPS instruction encoding ({:08X})\n", opcode, instruction);
                return []() {};
            default:
                LOG_ERROR("Unimplemented or invalid I-type instruction! Instruction opcode: {:02X} - from hex MIPS instruction encoding ({:08X})\n", opcode, instruction);
                return []() {};
            } 
        }
        break;
    }
}
