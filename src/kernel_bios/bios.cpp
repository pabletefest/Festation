#include "bios.hpp"
#include "tty.hpp"
#include "cpu/psx_cw33300_cpu.hpp"
#include "cpu/cpu_masks_types_utils.hpp"
#include "utils/file_reader.hpp"

#include <filesystem>
#include <fstream>
#include <cassert>

static constexpr size_t R9 = festation::GprRegs::t1;
static constexpr size_t R4 = festation::GprRegs::a0;

namespace festation
{
    static constexpr const uint32_t BIOS_SIZE = 512 * 1024;
};

festation::KernelBIOS::KernelBIOS(MIPS_R3000A_Core& _cpu)
    : KernelBIOS(_cpu, std::filesystem::path(std::filesystem::current_path().string() + "/../../../res/bios/SCPH1001.BIN"))
{

}

festation::KernelBIOS::KernelBIOS(MIPS_R3000A_Core& _cpu, const std::filesystem::path &filename)
    : cpu(_cpu)
{
    loadBIOSROMFile(filename);
}

uint8_t festation::KernelBIOS::read8(uint32_t address)
{
    return biosROM[address];
}

uint16_t festation::KernelBIOS::read16(uint32_t address)
{
    return *(uint16_t*)&biosROM[address];
}

uint32_t festation::KernelBIOS::read32(uint32_t address)
{
    return *(uint32_t*)&biosROM[address];
}

void festation::KernelBIOS::write8(uint32_t address, uint8_t value)
{
    biosROM[address] = value;
}

void festation::KernelBIOS::write16(uint32_t address, uint16_t value)
{
    *(uint16_t*)&biosROM[address] = value;
}

void festation::KernelBIOS::write32(uint32_t address, uint32_t value)
{
    *(uint32_t*)&biosROM[address] = value;
}

bool festation::KernelBIOS::loadBIOSROMFile(const std::filesystem::path &filename)
{
    std::ifstream file;

    file.open(filename, std::ios::in | std::ios::binary);

    if (file.is_open())
    {
        biosROM = readFile<uint8_t>(std::filesystem::path(filename));

        assert((biosROM.size() == BIOS_SIZE) && "Provided BIOS ROM file doesn't match proper PSX BIOS file size (512KB)!");

        return true;
    } 

    return false;
}

void festation::KernelBIOS::checkKernerlTTYOutput()
{
    uint32_t masked_pc = cpu.getCPURegs().pc & 0x1FFFFFFF; // Only interested on the first 29 bits (any MIPS memory region)
    uint32_t r9FunctNumber = cpu.getCPURegs().gpr_regs[R9];

    if ((masked_pc == 0x000000A0 && r9FunctNumber == 0x3C) || (masked_pc == 0x000000B0 && r9FunctNumber == 0x3D))
    {
        kernel_putchar(cpu.getCPURegs().gpr_regs[R4] & 0x000000FF);
    }
}
