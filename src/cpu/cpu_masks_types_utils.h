#pragma once

#include <cstdint>
#include <variant>
#include <functional>
#include <utility>

namespace festation
{
    enum GprRegs
    {
        zero = 0,   // R0
        at,         // R1
        v0,         // R2
        v1,         // R3
        a0,         // R4
        a1,         // R5
        a2,         // R6
        a3,         // R7
        t0,         // R8
        t1,         // R9
        t2,         // R10
        t3,         // R11
        t4,         // R12
        t5,         // R13
        t6,         // R14
        t7,         // R15
        s0,         // R16
        s1,         // R17
        s2,         // R18
        s3,         // R19
        s4,         // R20
        s5,         // R21
        s6,         // R22
        s7,         // R23
        t8,         // R24
        t9,         // R25
        k0,         // R26
        k1,         // R27
        gp,         // R28
        sp,         // R39
        fp,         // R30
        s8 = fp,    // R30 (fp or s8)
        ra,         // R31
        COUNT       // REGS #
    };

    enum EncodingType
    {
        REGISTER,
        IMMEDIATE,
        JUMP
    };

    enum InstructionBitsMask
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
        FUNCTION_MASK = 0x3F,
        R_TYPE_OPCODE_PATTERN = 0,
        J_TYPE_OPCODE_MASK = 0b000111 ,
        J_TYPE_OPCODE_PATTERN = 2 // or higher opcode value
    };

    enum class SrcRegs
    {
        RT,
        RS
    };

    using shift_amount_t = uint8_t;
    using immed16_t = uint16_t;
    using j_immed26_offset_t = uint32_t;

    using RTypeInstructionPtr = std::function<void(uint8_t, uint8_t, uint8_t, uint8_t)>;
    using ITypeInstructionPtr = std::function<void(uint8_t, uint8_t, uint16_t)>;
    using JTypeInstructionPtr = std::function<void(uint32_t)>;

    using RTypeInstructionEncoding = std::tuple<RTypeInstructionPtr, GprRegs, GprRegs, GprRegs, shift_amount_t>;
    using ITypeInstructionEncoding = std::tuple<ITypeInstructionPtr, GprRegs, GprRegs, immed16_t>;
    using JTypeInstructionEncoding = std::tuple<JTypeInstructionPtr, j_immed26_offset_t>;

    using InstructionTypeVariant = std::variant<RTypeInstructionEncoding, ITypeInstructionEncoding, JTypeInstructionEncoding>;
    using InstructionType = std::pair<EncodingType, InstructionTypeVariant>;

    static inline constexpr uint8_t getInstOpcode(uint32_t instruction)
    {
        return (instruction >> OPCODE_SHIFT_AMOUNT) & OPCODE_MASK;
    }

    template<EncodingType ENCODING>
    static inline constexpr uint8_t getInstDestRegEncoding(uint32_t instruction)
    {
        uint8_t rd = 0;

        if constexpr (ENCODING == EncodingType::REGISTER)
        {
            rd = (instruction >> R_TYPE_SHIFT_AMOUNT) & REG_MASK;
        }
        else if constexpr (ENCODING == EncodingType::IMMEDIATE)
        {
            rd = (instruction >> I_TYPE_SHIFT_AMOUNT) & REG_MASK; // rt is dest
        }

        return rd;
    }

    template<EncodingType ENCODING = EncodingType::IMMEDIATE, SrcRegs SRC = SrcRegs::RT>
    static inline constexpr uint8_t getInstSrcRegEncoding(uint32_t instruction)
    {
        uint8_t src = 0;

        if constexpr (ENCODING == EncodingType::REGISTER)
        {
            if constexpr (SRC == SrcRegs::RS)
            {
                src = (instruction >> (R_TYPE_SHIFT_AMOUNT + RS_MASK_SHIFT)) & REG_MASK;
            }
            else if constexpr(SRC == SrcRegs::RT)
            {
                src = (instruction >> (R_TYPE_SHIFT_AMOUNT + RT_MASK_SHIFT)) & REG_MASK;
            }
        }
        else if constexpr (ENCODING == EncodingType::IMMEDIATE)
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
        return instruction & FUNCTION_MASK;
    }
};
