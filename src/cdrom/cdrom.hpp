#pragma once

#include <cstdint>
#include <array>
#include <cstring>

namespace festation {
    class CdromDrive {
    public:
        CdromDrive();
        ~CdromDrive();

        auto read8(uint32_t address) -> uint8_t;
        auto write8(uint32_t address, uint8_t value) -> void;

    private:
        constexpr static size_t BUFFER_SIZE = 16ull;

        /** @todo Move this container to its own utility file */
        template<size_t N>
        class FifoCircularBuffer {
            std::array<uint8_t, N> buffer;
            uint8_t currentIndex;

            inline auto next() -> uint8_t { 
                uint8_t item = buffer[currentIndex];
                currentIndex = (currentIndex + 1) & (BUFFER_SIZE - 1);
                return item;
            }

            inline auto drain() -> void {
                std::memset(buffer.data(), 0, N);
                currentIndex = 0;
            }
        };
        struct CdromRegisters {
            union {
                struct {
                    uint8_t RA : 2;
                    uint8_t ADPBUSY : 1;
                    uint8_t PRMEMPT : 1;
                    uint8_t PRMWRDY : 1;
                    uint8_t RSLRRDY : 1;
                    uint8_t DRQSTS : 1;
                    uint8_t BUSYSTS : 1;
                };

                uint8_t raw;
            } HSTS{};

            uint8_t COMMAND{};
            FifoCircularBuffer<BUFFER_SIZE> PARAMETERS{};

            union {
                struct {
                    uint8_t reserved : 5;
                    uint8_t SMEN : 1;
                    uint8_t BFWR : 1;
                    uint8_t BFRD : 1;
                };
                
                uint8_t raw;
            } HCHPCTL{};

            uint16_t RDDATA{};

            FifoCircularBuffer<BUFFER_SIZE> RESULT{};

            union {
                struct {
                    uint8_t INTSTS : 3;
                    uint8_t BFEMPT : 1;
                    uint8_t BFWRDY : 1;
                    uint8_t reserved : 3;
                };
                
                uint8_t raw;
            } HINTSTS{};

            union {
                struct {
                    uint8_t ENINT : 3;
                    uint8_t ENBFEMPT : 1;
                    uint8_t ENBFWRDY : 1;
                    uint8_t reserved : 3;
                };
                
                uint8_t raw;
            } HINTMSK{};

            union {
                struct {
                    uint8_t CLRINT : 3;
                    uint8_t CLRBFEMPT : 1;
                    uint8_t CLRBFWRDY : 1;
                    uint8_t SMADPCLR : 1;
                    uint8_t CLRPRM : 1;
                    uint8_t CHPRST : 1;
                };
                
                uint8_t raw;
            } HCLRCTL{};

            union {
                struct {
                    uint8_t ADPMUTE : 1;
                    uint8_t reserved1 : 4;
                    uint8_t CHNGATV : 1;
                    uint8_t reserved2 : 2;
                };
                
                uint8_t raw;
            } ADPCTL{};

            union {
                struct {
                    uint8_t SM : 1;
                    uint8_t reserved1 : 1;
                    uint8_t FS : 1;
                    uint8_t reserved2 : 1;
                    uint8_t BITLNGTH : 1;
                    uint8_t reserved3 : 1;
                    uint8_t EMPHASIS : 1;
                    uint8_t reserved4 : 1;
                };
                
                uint8_t raw;
            } CI{};
        } m_regs;
    };
};
