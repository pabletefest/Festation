#include "gpu.hpp"
#include "gpu_commands.h"
#include "utils/logger.hpp"

#include <utility>

festation::PsxGpu::PsxGpu()
    : GPUREAD({}), GPUSTAT({}), m_commandState(GpuCommandsState::WaitingForCommand),
        m_currentCmdArg(1), m_commandsFIFO({})
{
    reset();
}

festation::PsxGpu::~PsxGpu()
{
}

void festation::PsxGpu::reset()
{
    GPUSTAT.raw = 0x14802000;
    GPUSTAT.readyToSendVRAMtoCPU = 1; // TODO: TEMP
}

uint32_t festation::PsxGpu::read32(uint32_t address)
{
    switch(address) {
    case 0x1F801810:
        return GPUREAD.raw;
    case 0x1F801814:
        return GPUSTAT.raw;
    default:
        std::unreachable();
    }
}

void festation::PsxGpu::write32(uint32_t address, uint32_t value)
{
    switch(address) {
    case 0x1F801810:
        switch (m_commandState)
        {
        case GpuCommandsState::WaitingForCommand:
            parseCommandGP0(value);
            break;
        case GpuCommandsState::ProcessingRectangleCommand:
            processGP0RectangleCmd(value);
            break;
        default:
            break;
        }
        break;
    case 0x1F801814:
        break;
    default:
        std::unreachable();
    }
}

void festation::PsxGpu::parseCommandGP0(uint32_t commandWord)
{
    uint8_t command = commandWord >> 29;

    switch(command)
    {
    default:
        LOG_DEBUG("Unimplemented GP0 GPU command (0b{:b})", command);
        break;
    }
}

void festation::PsxGpu::processGP0RectangleCmd(uint32_t commandWord)
{
}

void festation::PsxGpu::parseCommandGP1(uint32_t commandWord)
{
    uint8_t command = commandWord >> 24;
    uint32_t parameter = commandWord & 0x00FFFFFF;

    switch (command)
    {
    default:
        LOG_DEBUG("Unimplemented GP1 GPU command ({:x}h)", command);
        break;
    }
}

void festation::PsxGpu::processResetGpuCmd()
{
}

void festation::PsxGpu::processResetCommandBufferCmd()
{
    std::memset(m_commandsFIFO.data(), 0, m_commandsFIFO.size());
}

void festation::PsxGpu::processAckGpuIntCmd()
{
}

void festation::PsxGpu::processDisplayEnableCmd()
{
}

void festation::PsxGpu::processDmaDirectionDataRequestCmd()
{
}

void festation::PsxGpu::processStartDisplayAreaCmd()
{
}

void festation::PsxGpu::processHorizontalDisplayRangeCmd()
{
}

void festation::PsxGpu::processVerticalDisplayRangeCmd()
{
}

void festation::PsxGpu::processDisplayModeCmd()
{
}

void festation::PsxGpu::processSetVramSizeCmd()
{
}

void festation::PsxGpu::processReadGpuInternalRegCmd()
{
}
