#include <dma/dma_control.hpp>
#include "dma_control.hpp"
#include "dma_type.hpp"
#include "interrupts/interrupts.hpp"

#include <utility>

festation::DmaControl::DmaControl(PSXSystem& system, InterruptsHandler& interruptsHandler, Scheduler& scheduler)
    : m_system(system), m_interruptsHandler(interruptsHandler)
{
    m_channels[0] = std::make_unique<Dma0MdecIn>(*this, system, scheduler);
    m_channels[1] = std::make_unique<Dma1MdecOut>(*this, system, scheduler);
    m_channels[2] = std::make_unique<Dma2Gpu>(*this, system, scheduler);
    m_channels[3] = std::make_unique<Dma3Cdrom>(*this, system, scheduler);
    m_channels[4] = std::make_unique<Dma4Spu>(*this,system, scheduler);
    m_channels[5] = std::make_unique<Dma5Pio>(*this, system, scheduler);
    m_channels[6] = std::make_unique<Dma6Otc>(*this, system, scheduler);

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

auto festation::DmaControl::write8(uint32_t address, uint8_t value) -> void
{
    switch(address)
    {
    case 0x1F8010F4: // Check DICR write bug
        write32(address, static_cast<uint32_t>(value));
        break;
    case 0x1F8010F5: // Check DICR write bug
        write32(address, static_cast<uint32_t>(value << 8));
        break;
    case 0x1F8010F6: // Check DICR write bug
        write32(address, static_cast<uint32_t>(value << 16));
        break;
    case 0x1F8010F7: // Check DICR write bug
        write32(address, static_cast<uint32_t>(value << 24));
        break;
    default:
        std::unreachable();
    }
}

auto festation::DmaControl::write16(uint32_t address, uint16_t value) -> void
{
    switch(address)
    {
    case 0x1F8010F4: // Check DICR write bug
        write32(address, static_cast<uint32_t>(value));
        break;
    case 0x1F8010F5: // Check DICR write bug
        write32(address, static_cast<uint32_t>(value << 8));
        break;
    case 0x1F8010F6: // Check DICR write bug
        write32(address, static_cast<uint32_t>(value << 16));
        break;
    case 0x1F8010F7: // Check DICR write bug
        write32(address, static_cast<uint32_t>(value << 24));
        break;
    default:
        std::unreachable();
    }
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
        DICR.channelsCompletionInterruptsControl = value & 0x7F;
        DICR.unused = 0;
        DICR.busErrorFlag = (value >> 15) & 1;
        DICR.channelsInterruptMask = (value >> 16) & 0x7F;
        DICR.masterChannelInterruptEnable = (value >> 23) & 1;
        DICR.channelsInterruptFlags &= ~((value >> 24) & 0x7F); 

        if (DICR.busErrorFlag || (DICR.masterChannelInterruptEnable && (DICR.masterInterruptFlag > 0))) {

            if (DICR.masterInterruptFlag == 0) {
                /** @brief Only raise IRQ3 flag when transitioning from 0 to 1 */
                m_interruptsHandler.setInterruptSource(InterruptSource::DmaSrc);
            }

            DICR.masterInterruptFlag = 1;
        }
        else {
            DICR.masterInterruptFlag = 0;
        }
        break;
    default:
        size_t channelId = ((address >> 4) & 0xFu) - 8u;
        m_channels[channelId]->write32(address, value);
    }
}

auto festation::DmaControl::setIntFlag(festation::DmaType type) -> void
{
    size_t dmaIndex{};

    switch (type)
    {
    case festation::DmaType::MDECin:
        dmaIndex = 0;
        break;
    case festation::DmaType::MDECout:
        dmaIndex = 1;
        break;
    case festation::DmaType::GPU:
        dmaIndex = 2;
        break;
    case festation::DmaType::CDROM:
        dmaIndex = 3;
        break;
    case festation::DmaType::SPU:
        dmaIndex = 4;
        break;
    case festation::DmaType::PIO:
        dmaIndex = 5;
        break;
    case festation::DmaType::OTC:
        dmaIndex = 6;
        break;
    default:
        std::unreachable();
    }

    if (((DICR.channelsInterruptMask >> dmaIndex) & 1) && DICR.masterChannelInterruptEnable) {
        DICR.channelsInterruptFlags |= (1 << dmaIndex);
    }
}
