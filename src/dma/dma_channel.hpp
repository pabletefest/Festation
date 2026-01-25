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

            };

            uint32_t raw;
        } Dx_MADR;

        union DmaBlockControl {
            struct {

            };
            
            uint32_t raw;
        } Dx_BCR;

        union DmaChannelControl {
            struct {

            };
            
            uint32_t raw;
        } Dx_CHCR;
    };
};