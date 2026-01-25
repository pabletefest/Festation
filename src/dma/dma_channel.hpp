#pragma once

#include <cstdint>

namespace festation {
    class DmaChannel {
    public:
        DmaChannel();
        ~DmaChannel();

        uint32_t read32(uint32_t address);
        void write32(uint32_t address, uint32_t value);

    private:
        union DmaBaseAddress {
            struct {
                uint32_t startMemoryAddress : 24;
                uint32_t unused : 8;
            };

            uint32_t raw;
        } D_MADR;

        union DmaBlockControl {
            struct BcrSyncMode0 {
                uint32_t wordsNumber : 16;
                uint32_t unused : 16;
            };

            struct BcrSyncMode1 {
                uint32_t blockSize : 16;
                uint32_t blocksAmount : 16;
            };

            struct BcrSyncMode2 {
                uint32_t unused;
            };
            
            uint32_t raw;
        } D_BCR;

        union DmaChannelControl {
            struct {
                uint32_t transferDirection : 1;
                uint32_t madrIncrementPerStep : 1;
                uint32_t unused : 6;
                uint32_t modeEffect : 1; /* For more info about bit 8 check https://psx-spx.consoledev.net/dmachannels/ */ 
                uint32_t transferSyncMode : 2;
                uint32_t unused2 : 5;
                uint32_t choppingDmaWindowSize : 3;
                uint32_t unused3 : 1;
                uint32_t choppingCpuWindowSize : 3;
                uint32_t unused4 : 1;
                uint32_t startTransfer : 1;
                uint32_t unused5 : 3;
                uint32_t forceTransferStartWithoutDREQWaiting : 1;
                uint32_t modeEffect2 : 1; /* For more info about bit 29 check https://psx-spx.consoledev.net/dmachannels/ */ 
                uint32_t performBusSnooping : 1;
                uint32_t unused6 : 1;
            };
            
            uint32_t raw;
        } D_CHCR;
    };
};