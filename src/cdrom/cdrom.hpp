#pragma once

#include <cstdint>

namespace festation {
    class CDROM {
    public:
        CDROM();
        ~CDROM();

        auto read8(uint32_t address) -> uint8_t;
        auto read16(uint32_t address) -> uint16_t;
        auto write8(uint32_t address, uint8_t value) -> void;

    private:
        struct CdromRegisters {
            uint8_t HSTS;
            uint8_t COMMAND;
            uint8_t PARAMETER;
            uint8_t HCHPCTL;
            uint16_t RDDATA;
            uint8_t RESULT;
            uint8_t HINTSTS;
            uint8_t HINTMSK;
            uint8_t HCLRCTL;
        } m_regs;
    };
};
