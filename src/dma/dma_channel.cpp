#include <dma/dma_channel.hpp>

festation::DmaChannel::DmaChannel()
    : D_MADR({}), D_BCR({}), D_CHCR({})
{
}

festation::DmaChannel::~DmaChannel()
{
}

uint32_t festation::DmaChannel::read32(uint32_t address)
{
    return 0;
}

void festation::DmaChannel::write32(uint32_t address, uint32_t value)
{
}
