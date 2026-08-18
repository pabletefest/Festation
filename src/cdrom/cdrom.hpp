#pragma once

#include "interrupts/interrupts.hpp"
#include "scheduler/scheduler.hpp"

#include <cstdint>
#include <array>
#include <cstring>
#include <vector>

namespace festation {
    enum CdromInterruptType {
        CDROM_INT0_NO_INTR,
        CDROM_INT1_DATA_READY,
        CDROM_INT2_COMPLETE,
        CDROM_INT3_ACKNOWLEDGE,
        CDROM_INT4_DATA_END,
        CDROM_INT5_DISK_ERROR,
        CDROM_INT6,
        CDROM_INT7,
    };

    class CdromDrive {
    public:
        CdromDrive(InterruptsHandler& intrHndRef, Scheduler& scheduler);
        ~CdromDrive();

        auto read8(uint32_t address) -> uint8_t;
        auto write8(uint32_t address, uint8_t value) -> void;
    
    private:
        auto decodeCommand() -> void;
        auto processNopCmd() -> void;
        auto processSetlocCmd() -> void;
        auto processReadNCmd() -> void;
        auto processSetmodeCmd() -> void;
        auto processSeekLCmd() -> void;
        auto processBiosVersionCmd() -> void;
        auto processGetIdCmd() -> void;

        auto isInterrupt() const -> bool;

    private:
        constexpr static size_t BUFFER_SIZE = 16ull;

        /** @todo Move this container to its own utility file */
        template<size_t N>
        class FifoCircularBuffer {
        public:
            inline auto next() -> uint8_t { 
                uint8_t item = buffer[lastRequestedIndex];
                lastRequestedIndex = (lastRequestedIndex + 1) & (BUFFER_SIZE - 1);
                return item;
            }

            inline auto append(uint8_t value) -> void { 
                buffer[nextFreeIndex] = value;
                nextFreeIndex = (nextFreeIndex + 1) & (BUFFER_SIZE - 1);
            }

            inline auto drain() -> void {
                // std::memset(buffer.data(), 0, N);
                lastRequestedIndex = 0;
                nextFreeIndex = 0;
            }

        private:
            std::array<uint8_t, N> buffer;
            uint8_t lastRequestedIndex;
            uint8_t nextFreeIndex;
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

        union {
            struct {
                uint8_t error : 1;
                uint8_t spindleMotor : 1;
                uint8_t seekError : 1;
                uint8_t idError : 1;
                uint8_t shellOpen : 1;
                uint8_t read : 1;
                uint8_t seek : 1;
                uint8_t play : 1;
            };

            uint8_t raw;
        } m_internalStatusCode{};

        struct SeekTargetBCD {
            uint8_t minutes;
            uint8_t seconds;
            uint8_t sector;
        } m_seekTargetBCD{};

        uint32_t m_lda{};
        
        union {
            struct {
                uint8_t CDDA : 1;
                uint8_t autoPause : 1;
                uint8_t report : 1;
                uint8_t XA_filter : 1;
                uint8_t ignoreBit : 1;
                uint8_t sectorSize : 1;
                uint8_t XA_ADPCM : 1;
                uint8_t speed : 1;
            };

            uint8_t raw;
        } m_mode{};

        std::vector<uint8_t> m_sectorBlock;

        InterruptsHandler& m_interruptsHandler;
        Scheduler& m_scheduler;
    };
};
