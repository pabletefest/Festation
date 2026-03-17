#pragma once

#include <cstdint>

namespace festation {
    class PSXSystem;

    enum TransferDirection : uint32_t {
        DeviceToRam = 0,
        RamToDevice = 1,
    };

    enum AddressIncrement : uint32_t {
        Forward = 0,
        Backward = 1,
    };

    enum SyncMode : uint32_t {
        BurstMode = 0,
        SliceMode = 1,
        LinkedListMode = 2,
        Reserved = 3,
    };

    class DmaChannel {
    public:
        DmaChannel(PSXSystem& system);
        virtual ~DmaChannel();

        auto read32(uint32_t address) -> uint32_t;
        auto write32(uint32_t address, uint32_t value) -> void;

        auto setChannelEnable(bool isEnabled) -> void;
        auto isEnabled() const -> bool;

    protected:
        virtual auto startTransfer() -> void = 0;
        virtual auto modifyControlRegister(uint32_t value) -> void;
        auto endTransfer() -> void;

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
                TransferDirection transferDirection : 1;
                AddressIncrement madrIncrementPerStep : 1;
                uint32_t unused : 6;
                uint32_t modeEffectBit8 : 1; /* For more info about bit 8 check https://psx-spx.consoledev.net/dmachannels/ */ 
                SyncMode transferSyncMode : 2;
                uint32_t unused2 : 5;
                uint32_t choppingDmaWindowSize : 3;
                uint32_t unused3 : 1;
                uint32_t choppingCpuWindowSize : 3;
                uint32_t unused4 : 1;
                uint32_t startTransfer : 1;
                uint32_t unused5 : 3;
                uint32_t forceTransferStartWithoutDREQWaiting : 1;
                uint32_t modeEffectBit29 : 1; /* For more info about bit 29 check https://psx-spx.consoledev.net/dmachannels/ */ 
                uint32_t performBusSnooping : 1;
                uint32_t unused6 : 1;
            };
            
            uint32_t raw;
        } D_CHCR;

        PSXSystem& m_system;
        bool m_isEnabled;
    };

    class Dma0MdecIn : public DmaChannel {
    public:
        Dma0MdecIn(PSXSystem& system);
        virtual ~Dma0MdecIn();

    protected:
        auto startTransfer() -> void override;
    };

    class Dma1MdecOut : public DmaChannel {
    public:
        Dma1MdecOut(PSXSystem& system);
        virtual ~Dma1MdecOut();

    protected:
        auto startTransfer() -> void override;
    };

    class Dma2Gpu : public DmaChannel {
    public:
        Dma2Gpu(PSXSystem& system);
        virtual ~Dma2Gpu();
    
    protected:
        auto startTransfer() -> void override;
    };

    class Dma3Cdrom : public DmaChannel {
    public:
        Dma3Cdrom(PSXSystem& system);
        virtual ~Dma3Cdrom();

    protected:
        auto startTransfer() -> void override;
    };

    class Dma4Spu : public DmaChannel {
    public:
        Dma4Spu(PSXSystem& system);
        virtual ~Dma4Spu();

    protected:
        auto startTransfer() -> void override;
    };

    class Dma5Pio : public DmaChannel {
    public:
        Dma5Pio(PSXSystem& system);
        virtual ~Dma5Pio();

    protected:
        auto startTransfer() -> void override;
    };

    class Dma6Otc : public DmaChannel {
    public:
        Dma6Otc(PSXSystem& system);
        virtual ~Dma6Otc();

    protected:
        auto startTransfer() -> void override;
        auto modifyControlRegister(uint32_t value) -> void override;
    };
};
