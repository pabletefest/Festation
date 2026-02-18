#pragma once            
#include "utils/logger.hpp"

#include <cstdint>
#include <utility>

namespace festation
{
    struct PSXRegs
    {
        uint32_t gpr_regs[32]{ 0 };  // General Porpouse Registers
        uint32_t pc{ 0 };            // Program Counter (usually ahead after fetching)
        uint32_t currentPC{ 0 };     // Current instruction PC
        uint32_t hi{ 0 };            // High part of mult/div opcodes results
        uint32_t lo{ 0 };            // Low part of mult/div opcodes results

    private:
        class LoadDelaySlot // Struct containing last latched loaded value from memory due to delay slot (cleared after being consumed) and corresponding register
        {
        private:
            uint32_t loadedValue = 0;
            uint8_t destReg = 0;
            bool isDelay = false;

            friend struct PSXRegs;
        }loadDelaySlotLatch;

        class BranchDelaySlot
        {
        private:
            uint32_t destAddr = 0;
            bool isDelay = false;

            friend struct PSXRegs;
        }branchDelaySlotLatch;

    public:
        constexpr inline bool isLoadDelaySlot() const
        {
            return loadDelaySlotLatch.isDelay;
        }

        constexpr inline void consumeLoadedData()
        {
           gpr_regs[loadDelaySlotLatch.destReg] = loadDelaySlotLatch.loadedValue;
           loadDelaySlotLatch.isDelay = false;
        }

        constexpr inline void storeDelayedData(uint32_t data, uint8_t reg)
        {
            loadDelaySlotLatch.loadedValue = data;
            loadDelaySlotLatch.destReg = reg;
            loadDelaySlotLatch.isDelay = true;
        }

        constexpr inline uint8_t getLoadReg() const
        {
            return loadDelaySlotLatch.destReg;
        }
        
        constexpr inline uint32_t getLoadValue() const
        {
            return loadDelaySlotLatch.loadedValue;
        }

        constexpr inline bool isBranchDelaySlot() const
        {
            return branchDelaySlotLatch.isDelay;
        }

        constexpr inline void performDelayedJump()
        {
            pc = branchDelaySlotLatch.destAddr;
            branchDelaySlotLatch.isDelay = false;
        }

        constexpr inline void storeDelayedJump(uint32_t destination)
        {
            branchDelaySlotLatch.destAddr = destination;
            branchDelaySlotLatch.isDelay = true;
        }
    };

    struct COP0SystemControlRegs
    {
        void setCop0RegisterValue(size_t index, uint32_t value) {
            //LOG_DEBUG("Setting COP0 reg {} to value 0x{:0X}", index, value);
            switch (index) {
            case 6:
            case 8:
            case 14:
            case 15:
                /* These registers are read-only from code POV*/
                break;
            case 12:
                SR.r = value;
                break;
            case 13: // Only bits 8-9 are writable
            {
                uint8_t prevBits = CAUSE.r & 0x300;
                CAUSE.r &= ~0x300;
                CAUSE.r |= prevBits | (value & 0x300);
            }
                break;
            default:
                //LOG_DEBUG("Unimplemented write to COP0 register {}!", index);
                cop0_regs[index] = value; // Generic buffer
                break;
            }
        }

        uint32_t getCop0RegisterValue(size_t index) const {
            switch (index) {
            case 6:
                //LOG_DEBUG("Read from COP0 reg TAR: 0x{:08X}", TAR);
                return TAR;
            case 8:
                //LOG_DEBUG("Read from COP0 reg BadVaddr: 0x{:08X}", BadVaddr);
                return BadVaddr;
            case 12:
                return SR.r;
            case 13:
                //LOG_DEBUG("Read from COP0 reg CAUSE: 0x{:08X}", CAUSE);
                return CAUSE.r;
            case 14:
                //LOG_DEBUG("Read from COP0 reg EPC: 0x{:08X}", EPC);
                return EPC;
            case 15:
                return PRID;
            default:
                //LOG_DEBUG("Unimplemented read from COP0 register {}!", index);
                return cop0_regs[index]; // Generic buffer
            }

            return 0;
        }

        // Public only to emulator internal code to avoid the code modify undesired bits
        // Code modifies COP0 regs via instructions using setter/getter
        uint32_t TAR;
        uint32_t BadVaddr;

        union {
            struct {
                uint32_t iec : 1;
                uint32_t kuc : 1;
                uint32_t iep : 1;
                uint32_t kup : 1;
                uint32_t ieo : 1;
                uint32_t kuo : 1;
                uint32_t unused : 2;
                uint32_t im : 8;
                uint32_t isc : 1;
                uint32_t swc : 1;
                uint32_t pz : 1;
                uint32_t cm : 1;
                uint32_t pe : 1;
                uint32_t ts : 1;
                uint32_t bev : 1;
                uint32_t unsued2 : 2;
                uint32_t re : 1;
                uint32_t unused3 : 2;
                uint32_t cu0 : 1;
                uint32_t cu1 : 1;
                uint32_t cu2 : 1;
                uint32_t cu3 : 1;
            };

            uint32_t r;
        } SR;

        union {
            struct {
                uint32_t unused : 2;
                uint32_t excCode : 5;
                uint32_t unused2 : 1;
                uint32_t sw : 2;
                uint32_t ip : 6;
                uint32_t unused3 : 12;
                uint32_t ce : 2;
                uint32_t bt : 1;
                uint32_t bd : 1;
            };

            uint32_t r;
        } CAUSE;

        uint32_t EPC;
        uint32_t PRID;

        uint32_t cop0_regs[32];
        //uint32_t NA_control_regs[32]; // None such
    };
};
