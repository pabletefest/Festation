#include <dma/dma_control.hpp>
#include "dma_control.hpp"

#include <algorithm>

festation::DmaControl::DmaControl(PSXSystem& system)
    : m_system(system)
{
    m_channels[0] = std::make_unique<Dma0MdecIn>(system);
    m_channels[1] = std::make_unique<Dma1MdecOut>(system);
    m_channels[2] = std::make_unique<Dma2Gpu>(system);
    m_channels[3] = std::make_unique<Dma3Cdrom>(system);
    m_channels[4] = std::make_unique<Dma4Spu>(system);
    m_channels[5] = std::make_unique<Dma5Pio>(system);
    m_channels[6] = std::make_unique<Dma6Otc>(system);

    reset();
}

festation::DmaControl::~DmaControl()
{
}

auto festation::DmaControl::reset() -> void
{
    DPCR.raw = 0x07654321u;
    DICR.raw = 0;
}

auto festation::DmaControl::read32(uint32_t address) -> uint32_t
{
    switch(address)
    {
    case 0x1F8010F0:
        return DPCR.raw;
        break;
    case 0x1F8010F4:
        return DICR.raw;
        break;
    default:
        size_t channelId = ((address >> 4) & 0xFu) - 8u;
        return m_channels[channelId]->read32(address);
    }

    return 0;
}

auto festation::DmaControl::write32(uint32_t address, uint32_t value) -> void
{
    switch(address)
    {
    case 0x1F8010F0:
        DPCR.raw = value;
        
        for (size_t channelId = 0; channelId < DMA_CHANNELS_COUNT; channelId++) {
            bool isEnabled = DPCR.raw & (1 << (channelId * 4 + 3));
            m_channels[channelId]->setChannelEnable(isEnabled);  
        }
        break;
    case 0x1F8010F4: // Check DICR write bug
        DICR.raw = value;
        break;
    default:
        size_t channelId = ((address >> 4) & 0xFu) - 8u;
        m_channels[channelId]->write32(address, value);
    }
}
