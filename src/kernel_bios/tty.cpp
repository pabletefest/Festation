#include "tty.hpp"
#include "psx_cpu_state.hpp"

#include <print>
#include <vector>

namespace festation
{
    static std::vector<char> bufferedOutputStream;

    static void _putchar(char chr)
    {
        bufferedOutputStream.push_back(chr);

        if (chr != '\n')
            return;

        bufferedOutputStream.push_back('\0');

        std::println("%s", bufferedOutputStream.data());

        bufferedOutputStream.clear();
    }
};

void festation::kernel_putchar(char chr)
{
    _putchar(chr);
}
