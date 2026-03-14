#pragma once

#include <cstdint>

namespace festation {
    class DmaChannel {
    public:
        DmaChannel();
        virtual ~DmaChannel();

        virtual auto startTransfer() -> void = 0;

        auto read32(uint32_t address) -> uint32_t;
        auto write32(uint32_t address, uint32_t value) -> void;

    protected:
        union DmaBaseAddress {
            struct {
                uint32_t startMemoryAddress : 24;
                uint32_t unused : 8;
            };

            uint32_t raw;
        } D_MADR;

        union DmaBlockControl {
            struct {
                uint32_t wordsNumber : 16;
                uint32_t unused : 16;
            } bcrSyncMode0;

            struct {
                uint32_t blockSize : 16;
                uint32_t blocksAmount : 16;
            } bcrSyncMode1;

            struct {
                uint32_t unused;
            } bcrSyncMode2;
            
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

    class Dma0MdecIn : public DmaChannel {
    public:
        Dma0MdecIn();
        virtual ~Dma0MdecIn();

        auto startTransfer() -> void override;
    };

    class Dma1MdecOut : public DmaChannel {
    public:
        Dma1MdecOut();
        virtual ~Dma1MdecOut();

        auto startTransfer() -> void override;
    };

    class Dma2Gpu : public DmaChannel {
    public:
        Dma2Gpu();
        virtual ~Dma2Gpu();

        auto startTransfer() -> void override;
    };

    class Dma3Cdrom : public DmaChannel {
    public:
        Dma3Cdrom();
        virtual ~Dma3Cdrom();

        auto startTransfer() -> void override;
    };

    class Dma4Spu : public DmaChannel {
    public:
        Dma4Spu();
        virtual ~Dma4Spu();

        auto startTransfer() -> void override;
    };

    class Dma5Pio : public DmaChannel {
    public:
        Dma5Pio();
        virtual ~Dma5Pio();

        auto startTransfer() -> void override;
    };

    class Dma6Otc : public DmaChannel {
    public:
        Dma6Otc();
        virtual ~Dma6Otc();

        auto startTransfer() -> void override;
    };
};
