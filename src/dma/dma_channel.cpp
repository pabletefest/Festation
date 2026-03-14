#include <dma/dma_channel.hpp>

#include <cassert>
#include "dma_channel.hpp"

festation::DmaChannel::DmaChannel()
    : D_MADR({}), D_BCR({}), D_CHCR({})
{
}

festation::DmaChannel::~DmaChannel()
{
}

auto festation::DmaChannel::read32(uint32_t address) -> uint32_t
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

    return 0xFF;
}

auto festation::DmaChannel::write32(uint32_t address, uint32_t value) -> void
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

        if (D_CHCR.startTransfer) {
            startTransfer();
        }
        break;
    default:
        assert(false);
    }
}

festation::Dma0MdecIn::Dma0MdecIn()
    : DmaChannel()
{
}

festation::Dma0MdecIn::~Dma0MdecIn()
{
}

auto festation::Dma0MdecIn::startTransfer() -> void
{
}

festation::Dma1MdecOut::Dma1MdecOut()
    : DmaChannel()
{
}

festation::Dma1MdecOut::~Dma1MdecOut()
{
}

auto festation::Dma1MdecOut::startTransfer() -> void
{
}

festation::Dma2Gpu::Dma2Gpu()
    : DmaChannel()
{
}

festation::Dma2Gpu::~Dma2Gpu()
{
}

auto festation::Dma2Gpu::startTransfer() -> void
{
}

festation::Dma3Cdrom::Dma3Cdrom()
    : DmaChannel()
{
}

festation::Dma3Cdrom::~Dma3Cdrom()
{
}

auto festation::Dma3Cdrom::startTransfer() -> void
{
}

festation::Dma4Spu::Dma4Spu()
    : DmaChannel()
{
}

festation::Dma4Spu::~Dma4Spu()
{
}

auto festation::Dma4Spu::startTransfer() -> void
{
}

festation::Dma5Pio::Dma5Pio()
    : DmaChannel()
{
}

festation::Dma5Pio::~Dma5Pio()
{
}

auto festation::Dma5Pio::startTransfer() -> void
{
}

festation::Dma6Otc::Dma6Otc()
    : DmaChannel()
{
}

festation::Dma6Otc::~Dma6Otc()
{
}

auto festation::Dma6Otc::startTransfer() -> void
{
}
