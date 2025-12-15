#include "tty.hpp"
#include "psx_cpu_state.hpp"

#include <stdio.h>
#include <vector>

#define R4 4

namespace festation
{
    extern PSXRegs r3000a_regs;

    static std::vector<char> bufferedOutputStream;

    static void _putchar(char chr)
    {
        bufferedOutputStream.push_back(chr);

        if (chr != '\n')
            return;

        bufferedOutputStream.push_back('\0');

        printf("%s", bufferedOutputStream.data());

        bufferedOutputStream.clear();
    }
};

void festation::kernel_putchar()
{
    char chrParameter = r3000a_regs.gpr_regs[R4] & 0x000000FF;
    _putchar(chrParameter);
}
