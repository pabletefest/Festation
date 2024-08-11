#include "psx_cpu.h"
#include "mips_r3000a_opcodes.h"

#include <cstring>
#include <stdio.h>

namespace festation
{
    #define INSTRUCTION_SIZE 4

    PSXRegs r3000a_regs;
};

festation::MIPS_R3000A::MIPS_R3000A()
    : tempRAM(2 * 1024 * 1024)
{
    std::memset((void*) &r3000a_regs, 0, sizeof(PSXRegs));
}

uint8_t festation::MIPS_R3000A::read8(uint32_t address)
{
    return tempRAM[address];
}

uint16_t festation::MIPS_R3000A::read16(uint32_t address)
{
    return *(uint16_t*)&tempRAM.data()[address];
}

uint32_t festation::MIPS_R3000A::read32(uint32_t address)
{
    return *(uint32_t*)&tempRAM.data()[address];
}

void festation::MIPS_R3000A::write8(uint32_t address, uint8_t value)
{
    tempRAM[address] = value;
}

void festation::MIPS_R3000A::write16(uint32_t address, uint16_t value)
{
    *(uint16_t*)&tempRAM.data()[address] = value;
}

void festation::MIPS_R3000A::write32(uint32_t address, uint32_t value)
{
    *(uint32_t*)&tempRAM.data()[address] = value;
}

void festation::MIPS_R3000A::executeInstruction()
{
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
}

uint32_t festation::MIPS_R3000A::fetchInstruction()
{
    uint32_t instruction = read32(r3000a_regs.pc);

    r3000a_regs.pc += INSTRUCTION_SIZE;

    return instruction;
}

festation::InstructionType festation::MIPS_R3000A::decodeInstruction(uint32_t instruction)
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

festation::InstructionTypeVariant festation::MIPS_R3000A::decodeRFormat(uint32_t instruction)
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
        break;
    case 0x02:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            srl(_rd, _rt, _shift);
        }, rd, rt, rs, shift) };
        break;
    case 0x03:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            sra(_rd, _rt, _shift);
        }, rd, rt, rs, shift) };
        break;
    case 0x04:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            sllv(_rd, _rt, _rs);
        }, rd, rt, rs, shift) };
        break;
    case 0x06:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            srlv(_rd, _rt, _rs);
        }, rd, rt, rs, shift) };
        break;
    case 0x07:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            srav(_rd, _rt, _rs);
        }, rd, rt, rs, shift) };
        break;
    case 0x08:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            jr(_rs);
        }, rd, rt, rs, shift) };
        break;
    case 0x09:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            jalr(_rs);
        }, rd, rt, rs, shift) };
        break;
    case 0x0C:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            //syscall();
        }, rd, rt, rs, shift) };
        break;
    case 0x0D:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            //break();
        }, rd, rt, rs, shift) };
        break;
    case 0x10:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mfhi(_rd);
        }, rd, rt, rs, shift) };
        break;
    case 0x11:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mthi(_rs);
        }, rd, rt, rs, shift) };
        break;
    case 0x12:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mflo(_rd);
        }, rd, rt, rs, shift) };
        break;
    case 0x13:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mtlo(_rs);
        }, rd, rt, rs, shift) };
        break;
    case 0x18:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            mult(_rs, _rt);
        }, rd, rt, rs, shift) };
        break;
    case 0x19:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            multu(_rs, _rt);
        }, rd, rt, rs, shift) };
        break;
    case 0x1A:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            div(_rs, _rt);
        }, rd, rt, rs, shift) };
        break;
    case 0x1B:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            divu(_rs, _rt);
        }, rd, rt, rs, shift) };
        break;
    case 0x20:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            add(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
        break;
    case 0x21:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            addu(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
        break;
    case 0x22:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            sub(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
        break;
    case 0x23:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            subu(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
        break;
    case 0x24:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            _and(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
        break;
    case 0x25:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            _or(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
        break;
    case 0x26:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            _xor(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
        break;
    case 0x27:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            nor(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
        break;
    case 0x2A:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            slt(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
        break;
    case 0x2B:
        return { std::make_tuple([](reg_t _rd, reg_t _rt, reg_t _rs, shift_t _shift){
            sltu(_rd, _rs, _rt);
        }, rd, rt, rs, shift) };
        break;
    default:
        printf("Unimplemented or invalid R-type instruction! Function opcode: %02X - from hex MIPS instruction encoding (%08X)\n", function, instruction);
        return InstructionTypeVariant();
        break;
    }
}

festation::InstructionTypeVariant festation::MIPS_R3000A::decodeJFormat(uint32_t instruction)
{
    switch (getInstOpcode(instruction))
    {
    case 0x02:
        return { std::make_tuple([](j_immed26_t dest){
            j(dest);
        }, getInstAddress(instruction)) };
        break;
    case 0x03:
        return { std::make_tuple([](j_immed26_t dest){
            jal(dest);
        }, getInstAddress(instruction)) };
        break;
    default:
        printf("Unimplemented or invalid J-type instruction! Instruction opcode: %02X - from hex MIPS instruction encoding (%08X)\n", getInstOpcode(instruction), instruction);
        return InstructionTypeVariant();
        break;
    }
}

festation::InstructionTypeVariant festation::MIPS_R3000A::decodeIFormat(uint32_t instruction)
{
    return InstructionTypeVariant();
}
