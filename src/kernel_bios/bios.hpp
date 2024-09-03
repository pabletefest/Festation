#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace festation
{
    class KernelBIOS
    {
    public:
        KernelBIOS();
        KernelBIOS(const std::string& filename);
        ~KernelBIOS() = default;

        uint8_t read8(uint32_t address);
        uint16_t read16(uint32_t address);
        uint32_t read32(uint32_t address);
        
        void write8(uint32_t address, uint8_t value);
        void write16(uint32_t address, uint16_t value);
        void write32(uint32_t address, uint32_t value);

        bool loadBIOSROMFile(const std::string& filename);

        inline std::vector<uint8_t>& getBIOSData() { return biosROM; }

        void checkKernerlTTYOutput();

    // private:
    //     void checkKernelAFunctions();
    //     void checkKernelAFunctions();
    //     void checkKernelAFunctions();
    
    private:
        std::vector<uint8_t> biosROM;
    };
};
