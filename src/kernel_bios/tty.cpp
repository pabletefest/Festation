#include "tty.hpp"
#include "cpu/psx_cpu_state.hpp"
#include "utils/logger.hpp"

#include <print>
#include <vector>

namespace festation
{
    static std::vector<char> bufferedOutputStream;

    static void _putchar(char chr)
    {
        if (chr != '\n' && chr != '\0') {
            bufferedOutputStream.push_back(chr);
            return;
        }

        bufferedOutputStream.push_back('\0');

        LOG_KERNEL("{}", bufferedOutputStream.data());

        bufferedOutputStream.clear();
    }
};

void festation::kernel_putchar(char chr)
{
    _putchar(chr);
}
