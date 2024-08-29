#include "bios.hpp"

#include <filesystem>
#include <fstream>
#include <cassert>

namespace festation
{
    static constexpr const uint32_t BIOS_SIZE = 512 * 1024;
};

festation::KernelBIOS::KernelBIOS()
    : KernelBIOS(std::filesystem::canonical("/proc/self/exe").parent_path().string() + "/../bios/SCPH1001.BIN")
{

}

festation::KernelBIOS::KernelBIOS(const std::string &filename)
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

bool festation::KernelBIOS::loadBIOSROMFile(const std::string &filename)
{
    std::ifstream file;

    file.open(filename, std::ios::in | std::ios::binary);

    if (file.is_open())
    {
        // file.seekg(std::ios::end);
        // std::size_t fileSize = file.tellg();
        // file.seekg(std::ios::beg);

        // C++17 way
        std::uintmax_t fileSize = std::filesystem::file_size(filename);

        assert((fileSize == BIOS_SIZE) && "Provided BIOS ROM file doesn't match proper PSX BIOS file size (512KB)!");

        biosROM.resize(fileSize);

        file.read((char*)biosROM.data(), fileSize);

        return true;
    } 

    return false;
}
