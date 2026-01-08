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
                SR = value;
                break;
            case 13: // Only bits 8-9 are writable
            {
                uint8_t prevBits = CAUSE & 0x300;
                CAUSE &= ~0x300;
                CAUSE |= prevBits | (value & 0x300);
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
                return SR;
            case 13:
                //LOG_DEBUG("Read from COP0 reg CAUSE: 0x{:08X}", CAUSE);
                return CAUSE;
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
        uint32_t SR;
        uint32_t CAUSE;
        uint32_t EPC;
        uint32_t PRID;

        uint32_t cop0_regs[32];
        //uint32_t NA_control_regs[32]; // None such
    };
};
