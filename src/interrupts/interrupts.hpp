#pragma once

#include <cstdint>

namespace festation {
    enum InterruptSource {
        VBlankSrc = (1 << 0),
        GpuSrc = (1 << 1),
        CdromSrc = (1 << 2),
        DmaSrc = (1 << 3),
        Tmr0Src = (1 << 4),
        Tmr1Src = (1 << 5),
        Tmr2Src = (1 << 6),
        CtrlMemCardSrc = (1 << 7),
        SioSrc = (1 << 8),
        SpuSrc = (1 << 9),
        CtrlLightpenSrc = (1 << 10),
    };

    class InterruptsHandler {
    public:
        InterruptsHandler() = default;
        ~InterruptsHandler() = default;

        auto read32(uint32_t address) -> uint32_t;
        auto write32(uint32_t address, uint32_t value) -> void;
    
        constexpr auto setInterruptSource(InterruptSource source) -> void;

    private:
        union {
            struct {
                uint32_t VBLANK : 1;
                uint32_t GPU : 1;
                uint32_t CDROM : 1;
                uint32_t DMA : 1;
                uint32_t TMR0 : 1;
                uint32_t TMR1 : 1;
                uint32_t TMR2 : 1;
                uint32_t CtrlMemCard : 1;
                uint32_t SIO : 1;
                uint32_t SPU : 1;
                uint32_t CtrlLigthpen : 1;
                uint32_t unused : 5;
                uint32_t garbage : 16;
            };

            uint32_t raw;
        } I_STAT;

        union {
            struct {
                uint32_t VBLANK : 1;
                uint32_t GPU : 1;
                uint32_t CDROM : 1;
                uint32_t DMA : 1;
                uint32_t TMR0 : 1;
                uint32_t TMR1 : 1;
                uint32_t TMR2 : 1;
                uint32_t CtrlMemCard : 1;
                uint32_t SIO : 1;
                uint32_t SPU : 1;
                uint32_t CtrlLigthpen : 1;
                uint32_t unused : 5;
                uint32_t garbage : 16;
            };

            uint32_t raw;
        } I_MASK;
    };
};
