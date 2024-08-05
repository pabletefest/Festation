#pragma once

#include <cstdint>

namespace festation
{
    enum class ENCODING_TYPE
    {
        REGISTER,
        IMMEDIATE,
        JUMP
    };

    enum INSTRUCTION_BITS_MASK
    {
        OPCODE_SHIFT_AMOUNT = 26,
        R_TYPE_SHIFT_AMOUNT = 11,
        I_TYPE_SHIFT_AMOUNT = 16,
        OPCODE_MASK = 0x3F,
        RS_MASK_SHIFT = 10,
        RT_MASK_SHIFT = 5,
        //RD_MASK_SHIFT = 0, // Not needed because opcode type shift amount applied
        REG_MASK = 0X1F,
        SHIFT_MASK_BIT_LOCATION = 6,
        SHIFT_MASK = 0x1F,
        FUNCTION_MASK = 0x3F
    };

    enum class SRC_REG
    {
        RT,
        RS
    };

    static inline constexpr uint8_t getInstOpcode(uint32_t instruction)
    {
        return (instruction >> OPCODE_SHIFT_AMOUNT) & OPCODE_MASK;
    }

    template<ENCODING_TYPE ENCODING>
    static inline constexpr uint8_t getInstDestRegEncoding(uint32_t instruction)
    {
        uint8_t rd = 0;

        if constexpr (ENCODING == ENCODING_TYPE::REGISTER)
        {
            rd = (instruction >> R_TYPE_SHIFT_AMOUNT) & REG_MASK;
        }
        else if constexpr (ENCODING == ENCODING_TYPE::IMMEDIATE)
        {
            rd = (instruction >> I_TYPE_SHIFT_AMOUNT) & RD_MASK; // rt is dest
        }

        return rd;
    }

    template<ENCODING_TYPE ENCODING = ENCODING_TYPE::IMMEDIATE, SRC_REG SRC = SRC_REG::RT>
    static inline constexpr uint8_t getInstSrcRegEncoding(uint32_t instruction)
    {
        uint8_t src = 0;

        if constexpr (ENCODING == ENCODING_TYPE::REGISTER)
        {
            if constexpr (SRC == SRC_REG::RS)
            {
                src = (instruction >> (R_TYPE_SHIFT_AMOUNT + RS_MASK_SHIFT) & REG_MASK;
            }
            else if constexpr(SRC == SRC_REG::RT)
            {
                src = (instruction >> (R_TYPE_SHIFT_AMOUNT + RT_MASK_SHIFT)) & REG_MASK;
            }
        }
        else if constexpr (ENCODING == ENCODING_TYPE::IMMEDIATE)
        {
            src = (instruction >> (I_TYPE_SHIFT_AMOUNT + RT_MASK_SHIFT)) & REG_MASK; // rt is src
        }

        return src;
    }

    static inline constexpr uint8_t getInstShiftAmount(uint32_t instruction)
    {
        return (instruction >> SHIFT_MASK_BIT_LOCATION) & SHIFT_MASK;
    }

    static inline constexpr uint8_t getInstFunctionOperation(uint32_t instruction)
    {
        return instruction >> FUNCTION_MASK;
    }
};
