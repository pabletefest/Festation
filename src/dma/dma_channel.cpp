#include <dma/dma_channel.hpp>

#include <cassert>

festation::DmaChannel::DmaChannel()
    : D_MADR({}), D_BCR({}), D_CHCR({})
{
}

festation::DmaChannel::~DmaChannel()
{
}

uint32_t festation::DmaChannel::read32(uint32_t address)
{
    switch(address & 0xFu)
    {
    case 0x0:
        return D_MADR.raw;
    case 0x4:
        return D_BCR.raw;
    case 0x8:
        return D_CHCR.raw;
    default:
        assert(false);
    }
}

void festation::DmaChannel::write32(uint32_t address, uint32_t value)
{
    switch(address & 0xFu)
    {
    case 0x0:
        D_MADR.raw = 0xFF000000u | (value & 0x00FFFFFF);
        break;
    case 0x4:
        switch(D_CHCR.transferSyncMode)
        {
        case 0:
            // Check edge cases
            D_BCR.raw = value;           
            break;
        case 1:
            D_BCR.raw = value;
            break;
        case 2:
            D_BCR.raw = 0;
            break;
        case 3: // Reserved
            break;
        }
        break;
    case 0x8:
        D_CHCR.raw = value;
        break;
    default:
        assert(false);
    }
}
