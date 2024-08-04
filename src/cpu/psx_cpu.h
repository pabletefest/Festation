#pragma once

#include <cstdint>

class MIPS_R3000A
{
public:
    MIPS_R3000A() = default;
    ~MIPS_R3000A() = default;

    void clockCycles(uint32_t cycles);

private:
    uint64_t totalCyclesElapsed;
};