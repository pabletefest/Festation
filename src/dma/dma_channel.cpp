#include "dma_channel.hpp"
#include "psx_system.hpp"

#include <cassert>

festation::DmaChannel::DmaChannel(PSXSystem& system)
    : D_MADR({}), D_BCR({}), D_CHCR({}), m_system(system)
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

        if (D_CHCR.startTransfer && m_isEnabled) {
            startTransfer();
        }
        break;
    default:
        assert(false);
    }
}

auto festation::DmaChannel::setChannelEnable(bool isEnabled) -> void
{
    m_isEnabled = isEnabled;
}

auto festation::DmaChannel::isEnabled() const -> bool
{
    return m_isEnabled;
}

festation::Dma0MdecIn::Dma0MdecIn(PSXSystem& system)
    : DmaChannel(system)
{
}

festation::Dma0MdecIn::~Dma0MdecIn()
{
}

auto festation::Dma0MdecIn::startTransfer() -> void
{
}

festation::Dma1MdecOut::Dma1MdecOut(PSXSystem& system)
    : DmaChannel(system)
{
}

festation::Dma1MdecOut::~Dma1MdecOut()
{
}

auto festation::Dma1MdecOut::startTransfer() -> void
{
}

festation::Dma2Gpu::Dma2Gpu(PSXSystem& system)
    : DmaChannel(system)
{
}

festation::Dma2Gpu::~Dma2Gpu()
{
}

auto festation::Dma2Gpu::startTransfer() -> void
{
    assert(D_CHCR.transferSyncMode == 2);

    struct NodeHeader {
        uint32_t nextNodeAddress : 24;
        uint32_t wordsCount : 8;
    };

    std::function<void(uint32_t)> transferWordFn;

    if (D_CHCR.transferDirection == 1) {
        transferWordFn = [this](uint32_t address) {
            uint32_t word = this->m_system.read32(address);
            this->m_system.write32(0x1F801810, word);
        };
    }
    else {
        transferWordFn = [this](uint32_t address) {
            uint32_t word = this->m_system.read32(0x1F801810);
            this->m_system.write32(address, word);
        };
    }

    while (true) {
        uint32_t firstNodeWord = this->m_system.read32(D_MADR.startMemoryAddress & 0x00FFFFFF);
        uint32_t wordAddress = D_MADR.startMemoryAddress + 4;
        NodeHeader header;

        std::memcpy(&header, &firstNodeWord, sizeof(NodeHeader));

        while (header.wordsCount > 0) {
            transferWordFn(wordAddress);
            wordAddress += 4;
            header.wordsCount--;
        }

        /** @brief When nextNodeAddress is 0xFFFFFF, it marks the end of the DMA transfer (current node is transfered anyway)*/
        if (header.nextNodeAddress == 0xFFFFFF)
            break;

        D_MADR.startMemoryAddress = header.nextNodeAddress;
    }

    D_CHCR.startTransfer = 0;
}

festation::Dma3Cdrom::Dma3Cdrom(PSXSystem& system)
    : DmaChannel(system)
{
}

festation::Dma3Cdrom::~Dma3Cdrom()
{
}

auto festation::Dma3Cdrom::startTransfer() -> void
{
}

festation::Dma4Spu::Dma4Spu(PSXSystem& system)
    : DmaChannel(system)
{
}

festation::Dma4Spu::~Dma4Spu()
{
}

auto festation::Dma4Spu::startTransfer() -> void
{
}

festation::Dma5Pio::Dma5Pio(PSXSystem& system)
    : DmaChannel(system)
{
}

festation::Dma5Pio::~Dma5Pio()
{
}

auto festation::Dma5Pio::startTransfer() -> void
{
}

festation::Dma6Otc::Dma6Otc(PSXSystem& system)
    : DmaChannel(system)
{
}

festation::Dma6Otc::~Dma6Otc()
{
}

auto festation::Dma6Otc::startTransfer() -> void
{
    assert(D_CHCR.transferSyncMode == 0);

    uint32_t startAddress = D_MADR.startMemoryAddress & 0x00FFFFFF;
    uint32_t wordsCount = (D_BCR.bcrSyncMode0.wordsNumber > 0) ? D_BCR.bcrSyncMode0.wordsNumber : 0x10000;

    do {
        startAddress -= 4;
        uint32_t tableEntry = (wordsCount > 1) ? startAddress : 0x00FFFFFF;
        m_system.write32(startAddress, tableEntry);
        wordsCount--;
    } while (wordsCount > 0);

    D_CHCR.startTransfer = 0;
}
