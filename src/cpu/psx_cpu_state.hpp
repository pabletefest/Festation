#pragma once            

#include <cstdint>
#include <utility>

namespace festation
{
    struct PSXRegs
    {
        uint32_t gpr_regs[32]{ 0 };  // General Porpouse Registers
        uint32_t pc{ 0 };            // Program Counter
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
        uint32_t cop0_regs[32];
        uint32_t NA_control_regs[32]; // None such
    };
};
