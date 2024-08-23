#include "psx_cw33300_cpu.hpp"
#include "psx_system.hpp"
#include "mips_r3000a_opcodes.hpp"

#include <cstring>
#include <stdio.h>

namespace festation
{
    #define INSTRUCTION_SIZE 4

    PSXRegs r3000a_regs;
};

festation::MIPS_R3000A_Core::MIPS_R3000A_Core(PSXSystem* device)
    : system(device)
{
    std::memset((void*) &r3000a_regs, 0, sizeof(PSXRegs));
}

uint8_t festation::MIPS_R3000A_Core::read8(uint32_t address)
{
    return system->read8(address);
}

uint16_t festation::MIPS_R3000A_Core::read16(uint32_t address)
{
    return system->read16(address);
}

uint32_t festation::MIPS_R3000A_Core::read32(uint32_t address)
{
    return system->read32(address);
}

void festation::MIPS_R3000A_Core::write8(uint32_t address, uint8_t value)
{
    system->write8(address, value);
}

void festation::MIPS_R3000A_Core::write16(uint32_t address, uint16_t value)
{
    system->write16(address, value);
}

void festation::MIPS_R3000A_Core::write32(uint32_t address, uint32_t value)
{
    system->write32(address, value);
}

void festation::MIPS_R3000A_Core::executeInstruction()
{
    const bool isLoadDelayPending = r3000a_regs.isLoadDelaySlot();

    uint8_t instruction = fetchInstruction();

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

            instructionCodePtr(rd, rt, rs, shiftAmount);
        }
        break;
    case EncodingType::IMMEDIATE:
        {
            ITypeInstructionEncoding instructionEncoding = std::get<EncodingType::IMMEDIATE>(instructionTypePtr);
            auto instructionCodePtr = std::get<0>(instructionEncoding);
            reg_t rt = std::get<1>(instructionEncoding);
            reg_t rs = std::get<2>(instructionEncoding);
            immed16_t immed16 = std::get<3>(instructionEncoding);

            instructionCodePtr(rt, rs, immed16);
        }
        break;
    case EncodingType::JUMP:
        {
            JTypeInstructionEncoding instructionEncoding = std::get<EncodingType::JUMP>(instructionTypePtr);
            auto instructionCodePtr = std::get<0>(instructionEncoding);
            j_immed26_t immed26 = std::get<1>(instructionEncoding);

            instructionCodePtr(immed26);
        }
        break;
    default:
        break;
    }

    if (isLoadDelayPending)
        r3000a_regs.consumeLoadedData();
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
        return std::make_pair<EncodingType, InstructionTypeVariant>(EncodingType::IMMEDIATE, decodeIFormat(instruction));
    }
    else // I-FORMAT INSTRUCTION
    {
        return std::make_pair<EncodingType, InstructionTypeVariant>(EncodingType::JUMP, decodeJFormat(instruction));
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
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            sll(_rd, _rt, _shift);
        }, rd, rt, rs, shift) };
    case 0x02:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            srl(_rd, _rt, _shift);
        }, rd, rt, rs, shift) };
    case 0x03:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            sra(_rd, _rt, _shift);
        }, rd, rt, rs, shift) };
    case 0x04:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            sllv(_rd, _rt, _rs);
        }, rd, rt, rs, shift) };
    case 0x06:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            srlv(_rd, _rt, _rs);
        }, rd, rt, rs, shift) };
    case 0x07:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            srav(_rd, _rt, _rs);
        }, rd, rt, rs, shift) };
    case 0x08:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            jr(_rs);
        }, rd, rt, rs, shift) };
    case 0x09:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            jalr(_rs, _rd);
        }, rd, rt, rs, shift) };
    case 0x0C:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            //syscall();
        }, rd, rt, rs, shift) };
    case 0x0D:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            //break();
        }, rd, rt, rs, shift) };
    case 0x10:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mfhi(_rd);
        }, rd, rt, rs, shift) };
    case 0x11:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mthi(_rs);
        }, rd, rt, rs, shift) };
    case 0x12:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mflo(_rd);
        }, rd, rt, rs, shift) };
    case 0x13:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mtlo(_rs);
        }, rd, rt, rs, shift) };
    case 0x18:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mult(_rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x19:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            multu(_rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x1A:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            div(_rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x1B:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            divu(_rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x20:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            add(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x21:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            addu(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x22:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            sub(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x23:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            subu(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x24:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            _and(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x25:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            _or(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x26:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            _xor(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x27:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            nor(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x2A:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            slt(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    case 0x2B:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            sltu(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
    default:
        printf("Unimplemented or invalid R-type instruction! Function opcode: %02X - from hex MIPS instruction encoding (%08X)\n", function, instruction);
        return InstructionTypeVariant();
    }
}

festation::InstructionTypeVariant festation::MIPS_R3000A_Core::decodeJFormat(uint32_t instruction)
{
    switch (getInstOpcode(instruction))
    {
    case 0x02:
        return { std::make_tuple([](j_immed26_t dest){
            j(dest);
        }, getInstAddress(instruction)) };
    case 0x03:
        return { std::make_tuple([](j_immed26_t dest){
            jal(dest);
        }, getInstAddress(instruction)) };
    default:
        printf("Unimplemented or invalid J-type instruction! Instruction opcode: %02X - from hex MIPS instruction encoding (%08X)\n", getInstOpcode(instruction), instruction);
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
            return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t dest){
                bltz(_rs, dest);
            }, rt, rs, imm16) };
        case 0b00001:
            return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t dest){
                bgez(_rs, dest);
            }, rt, rs, imm16) };
        case 0b10000:
            return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t dest){
                bltzal(_rs, dest);
            }, rt, rs, imm16) };
        case 0b10001:
            return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t dest){
                bgezal(_rs, dest);
            }, rt, rs, imm16) };
        default:
            printf("Unimplemented or invalid I-type BcondZ instruction! rt bits: %02X - from hex MIPS instruction encoding (%08X)\n", rt, instruction);
            return InstructionTypeVariant();
        }
    case 0x04:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t dest){
            beq(_rs, _rt, dest);
        }, rt, rs, imm16) };
    case 0x05:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t dest){
            bne(_rs, _rt, dest);
        }, rt, rs, imm16) };
    case 0x06:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t dest){
            blez(_rs, dest);
        }, rt, rs, imm16) };
    case 0x07:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t dest){
            bgtz(_rs, dest);
        }, rt, rs, imm16) };
    case 0x08:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            addi(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x09:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            addiu(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x0A:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            slti(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x0B:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            sltiu(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x0C:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            andi(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x0D:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            ori(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x0E:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            xori(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x0F:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            lui(_rt, _imm16);
        }, rt, rs, imm16) };
    case 0x10:
        return InstructionTypeVariant();
    case 0x11:
        return InstructionTypeVariant();
    case 0x12:
        return InstructionTypeVariant();
    case 0x13:
        return InstructionTypeVariant();
    case 0x20:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            lb(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x21:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            lh(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x22:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            lwl(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x23:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            lw(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x24:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            lbu(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x25:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            lhu(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x26:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            lwr(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x28:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            sb(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x29:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            sh(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x2A:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            swl(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x2B:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            sw(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x2E:
        return { std::make_tuple([](reg_t _rt, reg_t _rs, immed16_t _imm16){
            swr(_rt, _rs, _imm16);
        }, rt, rs, imm16) };
    case 0x30:
        return InstructionTypeVariant();
    case 0x31:
        return InstructionTypeVariant();
    case 0x32:
        return InstructionTypeVariant();
    case 0x33:
        return InstructionTypeVariant();
    case 0x38:
        return InstructionTypeVariant();
    case 0x39:
        return InstructionTypeVariant();
    case 0x3A:
        return InstructionTypeVariant();
    case 0x3B:
        return InstructionTypeVariant();
    default:
        printf("Unimplemented or invalid I-type instruction! Instruction opcode: %02X - from hex MIPS instruction encoding (%08X)\n", opcode, instruction);
        return InstructionTypeVariant();
    }
}
