#pragma once
#include "psx_cpu.h"

namespace festation
{
    class PSXSystem
    {
    public:
        PSXSystem() = default;
        ~PSXSystem() = default;

        void runWholeFrame();

    private:
        MIPS_R3000A cpu;
    };
};
