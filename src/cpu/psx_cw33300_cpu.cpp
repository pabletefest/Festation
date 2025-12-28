#include "psx_cw33300_cpu.hpp"
#include "psx_system.hpp"
#include "mips_r3000a_opcodes.hpp"
#include "coprocessor_cp0_opcodes.hpp"
#include "exceptions_handling.hpp"
#include "utils/logger.hpp"
#include "memory_map_masks.hpp"

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

    return system->read32(address);
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
}

uint8_t festation::MIPS_R3000A_Core::executeInstruction()
{
    const bool isBranchDelayPending = r3000a_regs.isBranchDelaySlot();

    uint32_t instruction = fetchInstruction();

    //LOG_DEBUG(  "* Executing instruction: 0x{:08X}  at address 0x{:08X} *", instruction, r3000a_regs.pc - 4);

    InstructionType instructionType = decodeInstruction(instruction);

    EncodingType type = std::get<0>(instructionType);

    InstructionTypeVariant instructionTypePtr = std::get<1>(instructionType);

    switch (type)
    {
    case EncodingType::REGISTER:
        {
            RTypeInstructionEncoding instructionEncoding = std::get<EncodingType::REGISTER>(instructionTypePtr);
            auto instructionCodePtr = std::get<0>(instructionEncoding);
            reg_t rd = std::get<1>(instructionEncoding);
            reg_t rt = std::get<2>(instructionEncoding);
            reg_t rs = std::get<3>(instructionEncoding);
            shift_t shiftAmount = std::get<4>(instructionEncoding);

            instructionCodePtr(*this, rd, rt, rs, shiftAmount);
        }
        break;
    case EncodingType::IMMEDIATE:
        {
            ITypeInstructionEncoding instructionEncoding = std::get<EncodingType::IMMEDIATE>(instructionTypePtr);
            auto instructionCodePtr = std::get<0>(instructionEncoding);
            reg_t rt = std::get<1>(instructionEncoding);
            reg_t rs = std::get<2>(instructionEncoding);
            immed16_t immed16 = std::get<3>(instructionEncoding);

            instructionCodePtr(*this, rt, rs, immed16);
        }
        break;
    case EncodingType::JUMP:
        {
            JTypeInstructionEncoding instructionEncoding = std::get<EncodingType::JUMP>(instructionTypePtr);
            auto instructionCodePtr = std::get<0>(instructionEncoding);
            j_immed26_t immed26 = std::get<1>(instructionEncoding);

            instructionCodePtr(*this, immed26);
        }
        break;
    default:
        break;
    }

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
    return (cop0_state.cop0_regs[SR] & CACHE_ISOLATION_BIT_MASK) != 0;
}

uint32_t festation::MIPS_R3000A_Core::fetchInstruction()
{
    uint32_t instruction = read32(r3000a_regs.pc);

    r3000a_regs.pc += INSTRUCTION_SIZE;

    return instruction;
}

festation::InstructionType festation::MIPS_R3000A_Core::decodeInstruction(uint32_t instruction)
{
    uint8_t opcode = getInstOpcode(instruction);

    if (opcode == R_TYPE_OPCODE_PATTERN) // R-FORMAT INSTRUCTION or SPECIAL
    {
        return std::make_pair<EncodingType, InstructionTypeVariant>(EncodingType::REGISTER, decodeRFormat(instruction));
    }
    else if (opcode == 0x02 || opcode == 0x03) // J-FORMAT INSTRUCTION
    {
        return std::make_pair<EncodingType, InstructionTypeVariant>(EncodingType::JUMP, decodeJFormat(instruction));
    }
    else // I-FORMAT INSTRUCTION
    {
        return std::make_pair<EncodingType, InstructionTypeVariant>(EncodingType::IMMEDIATE, decodeIFormat(instruction));
    }
}

festation::InstructionTypeVariant festation::MIPS_R3000A_Core::decodeRFormat(uint32_t instruction)
{
    uint8_t function = getInstFunctionOperation(instruction);
    reg_t rd = getInstDestRegEncoding<EncodingType::REGISTER>(instruction);
    reg_t rt = getInstSrcRegEncoding<EncodingType::REGISTER, SrcRegs::RT>(instruction);
    reg_t rs = getInstSrcRegEncoding<EncodingType::REGISTER, SrcRegs::RS>(instruction);
    shift_t shift = getInstShiftAmount(instruction);

    switch(function)
    {
    case 0x00:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            sll(cpu, _rd, _rt, _shift);
        }, rd, rt, rs, shift) };
    case 0x02:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            srl(cpu, _rd, _rt, _shift);
        }, rd, rt, rs, shift) };
    case 0x03:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            sra(cpu, _rd, _rt, _shift);
        }, rd, rt, rs, shift) };
    case 0x04:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            sllv(cpu, _rd, _rt, _rs);
        }, rd, rt, rs, shift) };
    case 0x06:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            srlv(cpu, _rd, _rt, _rs);
        }, rd, rt, rs, shift) };
    case 0x07:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            srav(cpu, _rd, _rt, _rs);
        }, rd, rt, rs, shift) };
    case 0x08:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            jr(cpu, _rs);
        }, rd, rt, rs, shift) };
    case 0x09:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            jalr(cpu, _rs, _rd);
        }, rd, rt, rs, shift) };
    case 0x0C:
        return { std::make_tuple([instruction](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            syscall_break_code_t code = getSyscallBreakCode(instruction);

            syscall(cpu, code);
        }, rd, rt, rs, shift) };
    case 0x0D:
        return { std::make_tuple([instruction](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            syscall_break_code_t code = getSyscallBreakCode(instruction);

            _break(cpu, code);
        }, rd, rt, rs, shift) };
    case 0x10:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mfhi(cpu, _rd);
        }, rd, rt, rs, shift) };
    case 0x11:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mthi(cpu, _rs);
        }, rd, rt, rs, shift) };
    case 0x12:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mflo(cpu, _rd);
        }, rd, rt, rs, shift) };
    case 0x13:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mtlo(cpu, _rs);
        }, rd, rt, rs, shift) };
    case 0x18:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mult(cpu, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x19:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            multu(cpu, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x1A:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            div(cpu, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x1B:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            divu(cpu, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x20:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            add(cpu, _rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x21:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            addu(cpu, _rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x22:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            sub(cpu, _rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x23:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            subu(cpu, _rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x24:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            _and(cpu, _rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x25:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            _or(cpu, _rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x26:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            _xor(cpu, _rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x27:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            nor(cpu, _rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x2A:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            slt(cpu, _rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x2B:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            sltu(cpu, _rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    default:
        LOG_ERROR("Unimplemented or invalid R-type instruction! Function opcode: {:02X} - from hex MIPS instruction encoding ({:08X})\n", function, instruction);
        assert(false);
        return InstructionTypeVariant();
    }
}

festation::InstructionTypeVariant festation::MIPS_R3000A_Core::decodeJFormat(uint32_t instruction)
{
    switch (getInstOpcode(instruction))
    {
    case 0x02:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, j_immed26_t dest){
            j(cpu, dest);
        }, getInstAddress(instruction)) };
    case 0x03:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, j_immed26_t dest){
            jal(cpu, dest);
        }, getInstAddress(instruction)) };
    default:
        LOG_ERROR("Unimplemented or invalid J-type instruction! Instruction opcode: {:02X} - from hex MIPS instruction encoding ({:08X}\n", getInstOpcode(instruction), instruction);
        assert(false);
        return InstructionTypeVariant();
    }
}

festation::InstructionTypeVariant festation::MIPS_R3000A_Core::decodeIFormat(uint32_t instruction)
{
    uint8_t opcode = getInstOpcode(instruction);
    reg_t rt = getInstDestRegEncoding<EncodingType::IMMEDIATE>(instruction);
    reg_t rs = getInstSrcRegEncoding<EncodingType::IMMEDIATE, SrcRegs::RS>(instruction);
    immed16_t imm16 = getInstImmediate(instruction);

    switch (opcode)
    {
    case 0x01:
        switch (rt) // To be able to know which BcondZ instructions is, we look the encoding in rt reg field bits
        {
        case 0b00000:
            return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t dest){
                bltz(cpu, _rs, dest);
            }, rt, rs, imm16) };
        case 0b00001:
            return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t dest){
                bgez(cpu, _rs, dest);
            }, rt, rs, imm16) };
        case 0b10000:
            return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t dest){
                bltzal(cpu, _rs, dest);
            }, rt, rs, imm16) };
        case 0b10001:
            return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t dest){
                bgezal(cpu, _rs, dest);
            }, rt, rs, imm16) };
        default:
            LOG_ERROR("Unimplemented or invalid I-type BcondZ instruction! rt bits: {:02} - from hex MIPS instruction encoding ({:08X})\n", rt, instruction);
            assert(false);
            return InstructionTypeVariant();
        }
    case 0x04:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t dest){
            beq(cpu, _rs, _rt, dest);
        }, rt, rs, imm16) };
    case 0x05:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t dest){
            bne(cpu, _rs, _rt, dest);
        }, rt, rs, imm16) };
    case 0x06:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t dest){
            blez(cpu, _rs, dest);
        }, rt, rs, imm16) };
    case 0x07:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t dest){
            bgtz(cpu, _rs, dest);
        }, rt, rs, imm16) };
    case 0x08:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            addi(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x09:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            addiu(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x0A:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            slti(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x0B:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            sltiu(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x0C:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            andi(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x0D:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            ori(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x0E:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            xori(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x0F:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            lui(cpu, _rt, _imm16);
        }, rt, rs, imm16) };
    case 0x10: // COP0
        if (rs == 0b10000)
        {
            // We don't check last 6 bits because PS1 CPU doesn't have TLB
            return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
                rfe(cpu);
            }, rt, rs, imm16) };
        }
        else
        {
            reg_t rtcop0 = getInstSrcRegEncoding<EncodingType::REGISTER, SrcRegs::RT>(instruction);
            reg_t rdcop0 = getInstDestRegEncoding<EncodingType::REGISTER>(instruction);

            switch (rs) // We don't need to check for more opcodes on COP0
            {
            case 0b00000:
                return { std::make_tuple([rtcop0, rdcop0](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
                    mfc0(cpu, rtcop0, rdcop0);
                }, rt, rs, imm16) };
            case 0b00100:
                return { std::make_tuple([rtcop0, rdcop0](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
                    mtc0(cpu, rtcop0, rdcop0);
                }, rt, rs, imm16) };
            default:
                LOG_ERROR("Unimplemented or invalid COP0 instruction! Instruction opcode: {:02} - from hex MIPS instruction encoding ({:08X})\n", opcode, instruction);
                assert(false);
                return InstructionTypeVariant();
            }
        }
    case 0x11: // COP1
        assert(false);
        return InstructionTypeVariant();
    case 0x12: // COP2
        return InstructionTypeVariant();
    case 0x13: // COP3
        assert(false);
        return InstructionTypeVariant();
    case 0x20:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            lb(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x21:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            lh(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x22:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            lwl(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x23:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            lw(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x24:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            lbu(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x25:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            lhu(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x26:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            lwr(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x28:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            sb(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x29:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            sh(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x2A:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            swl(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x2B:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            sw(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x2E:
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            swr(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x30: // COP0
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            lwc0(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x31: // COP1
        assert(false);
        return InstructionTypeVariant();
    case 0x32: // COP2
        return InstructionTypeVariant();
    case 0x33: // COP3
        assert(false);
        return InstructionTypeVariant();
    case 0x38: // COP0
        return { std::make_tuple([](MIPS_R3000A_Core& cpu, reg_t _rt, reg_t _rs, immed16_t _imm16){
            swc0(cpu, _rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x39: // COP1
        assert(false);
        return InstructionTypeVariant();
    case 0x3A: // COP2
        return InstructionTypeVariant();
    case 0x3B: // COP3
        assert(false);
        return InstructionTypeVariant();
    default:
        LOG_ERROR("Unimplemented or invalid I-type instruction! Instruction opcode: {:02X} - from hex MIPS instruction encoding ({:08X})\n", opcode, instruction);
        assert(false);
        return InstructionTypeVariant();
    }
}
