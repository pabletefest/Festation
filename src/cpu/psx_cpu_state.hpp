#pragma once            

#include <cstdint>
#include <utility>

namespace festation
{
    struct PSXRegs
    {
        uint32_t gpr_regs[32];  // General Porpouse Registers
        uint32_t pc;            // Program Counter
        uint32_t hi;            // High part of mult/div opcodes results
        uint32_t lo;            // Low part of mult/div opcodes results

        class LoadDelaySlot // Struct containing last latched loaded value from memory due to delay slot (cleared after being consumed) and corresponding register
        {
        private:
            uint32_t loadedValue = 0;
            uint8_t destReg = 0;
            bool isDelay = false;

            friend class PSXRegs;
        }delaySlotLatch;

        constexpr inline bool isLoadDelaySlot()
        {
            return delaySlotLatch.isDelay;
        }

        constexpr inline void consumeLoadedData()
        {
           gpr_regs[delaySlotLatch.destReg] = delaySlotLatch.loadedValue;
           delaySlotLatch.isDelay = false;
        }

        constexpr inline void storeDelayedData(uint32_t data, uint8_t reg)
        {
            delaySlotLatch.loadedValue = data;
            delaySlotLatch.destReg = reg;
            delaySlotLatch.isDelay = true;
        }
    };
};
