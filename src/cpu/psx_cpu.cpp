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

    InstructionTypePtr instructionTypePtr = std::get<1>(instructionType);

    switch (type)
    {
    case EncodingType::REGISTER:
        {
            RTypeInstructionEncoding instructionEncoding = std::get<EncodingType::REGISTER>(instructionTypePtr);
            auto instructionCodePtr = std::get<0>(instructionEncoding);
            uint8_t rd = std::get<1>(instructionEncoding);
            uint8_t rt = std::get<2>(instructionEncoding);
            uint8_t rs = std::get<3>(instructionEncoding);
            uint8_t shiftAmount = std::get<4>(instructionEncoding);

            instructionCodePtr(rd, rt, rs, shiftAmount);
        }
        break;
    case EncodingType::IMMEDIATE:
        {
            ITypeInstructionEncoding instructionEncoding = std::get<EncodingType::IMMEDIATE>(instructionTypePtr);
            auto instructionCodePtr = std::get<0>(instructionEncoding);
            uint8_t rt = std::get<1>(instructionEncoding);
            uint8_t rs = std::get<2>(instructionEncoding);
            uint16_t immed16 = std::get<3>(instructionEncoding);

            instructionCodePtr(rt, rs, immed16);
        }
        break;
    case EncodingType::JUMP:
        {
            JTypeInstructionEncoding instructionEncoding = std::get<EncodingType::JUMP>(instructionTypePtr);
            auto instructionCodePtr = std::get<0>(instructionEncoding);
            uint32_t immed32 = std::get<1>(instructionEncoding);

            instructionCodePtr(immed32);
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

    if (opcode == R_TYPE_OPCODE_PATTERN) // R-FORMAT INSTRUCTION
    {
        return std::make_pair<EncodingType, InstructionTypePtr>(EncodingType::REGISTER, decodeRFormat(instruction));
    }
    else if ((opcode & J_TYPE_OPCODE_MASK) >= J_TYPE_OPCODE_PATTERN) // J-FORMAT INSTRUCTION
    {
        return std::make_pair<EncodingType, InstructionTypePtr>(EncodingType::IMMEDIATE, decodeIFormat(instruction));
    }
    else // I-FORMAT INSTRUCTION
    {
        return std::make_pair<EncodingType, InstructionTypePtr>(EncodingType::JUMP, decodeJFormat(instruction));
    }
}

festation::InstructionTypePtr festation::MIPS_R3000A::decodeRFormat(uint32_t instruction)
{
    return InstructionTypePtr();
}

festation::InstructionTypePtr festation::MIPS_R3000A::decodeJFormat(uint32_t instruction)
{
    return InstructionTypePtr();
}

festation::InstructionTypePtr festation::MIPS_R3000A::decodeIFormat(uint32_t instruction)
{
    return InstructionTypePtr();
}
