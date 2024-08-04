#pragma once            

#include <cstdint>

namespace festation
{
    enum GPR_REGS
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

    struct PSXRegs
    {
        uint32_t gpr_regs[32];  // General Porpouse Registers
        uint32_t pc;            // Program Counter
        uint32_t hi;            // High part of mult/div opcodes results
        uint32_t lo;            // Low part of mult/div opcodes results
    };
};
