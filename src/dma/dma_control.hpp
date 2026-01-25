#pragma once

#include "dma/dma_channel.hpp"

#include <cstdint>
#include <array>

namespace festation {
    inline constexpr uint8_t DMA_CHANNELS = 6u;

    class DmaControl {
    public:
        DmaControl();
        ~DmaControl();

        uint32_t read32(uint32_t address);
        void write32(uint32_t address, uint32_t value);

    private:
        union DmaControlRegister {
            struct {

            };

            uint32_t raw;
        } DPCR;

        union DmaInterruptRegister {
            struct {

            };
            
            uint32_t raw;
        } DICR;

        std::array<DmaChannel, DMA_CHANNELS> channels;
    };
};