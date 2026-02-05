#include "gpu.hpp"
#include "gpu_commands.h"
#include "utils/logger.hpp"

#include <utility>

festation::PsxGpu::PsxGpu()
    : GPUREAD({}), GPUSTAT({}), m_commandState(GpuCommandsState::WaitingForCommand),
        m_currentCmdArg(1), m_commandsFIFO({})
{
    processResetGpuCmd();
}

festation::PsxGpu::~PsxGpu()
{
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
        // parseCommandGP1(value);
        break;
    default:
        std::unreachable();
    }
}

void festation::PsxGpu::parseCommandGP0(uint32_t commandWord)
{
    uint8_t command = commandWord >> 29;
    uint8_t fullCmd = commandWord >> 24;

    switch(command)
    {
    case Gpu0Commands::PolygonPrimitive:
        break;
    case Gpu0Commands::LinePrimitive:
        break;
    case Gpu0Commands::RectanglePrimitive:
        break;
    case Gpu0Commands::VramToVramBlit:
        break;
    case Gpu0Commands::CpuToVramBlit:
        break;
    case Gpu0Commands::VramToCpuBlit:
        break;
    case Gpu0Commands::Misc:
    case Gpu0Commands::Environment:
        switch(fullCmd)
        {
        case Gpu0Commands::ClearCache:
            break;
        case Gpu0Commands::QuickRectangleFill:
            break;
        case Gpu0Commands::InterruptRequest:
            break;
        case Gpu0Commands::DrawMode:
            processGP0DrawModeCmd(commandWord);
            break;
        case Gpu0Commands::TextureWindow:
            break;
        case Gpu0Commands::SetDrawingAreaX1Y1:
            break;
        case Gpu0Commands::SetDrawingAreaX2Y2:
            break;
        case Gpu0Commands::SetDrawingOffset:
            break;
        case Gpu0Commands::MaskBitSetting:
            break;
        default:
            LOG_DEBUG("Unimplemented GP0 Misc/Env GPU command ({:x}h)", fullCmd);
            break;
        }
        break;
    default:
        LOG_DEBUG("Unimplemented GP0 GPU command (0b{:b})", command);
        break;
    }
}

void festation::PsxGpu::processGP0RectangleCmd(uint32_t parameter)
{
}

void festation::PsxGpu::processGP0DrawModeCmd(uint32_t parameter)
{
    GPUSTAT.raw = (GPUSTAT.raw & 0xFFFFFC00) | (parameter & 0x3FF);
    GPUSTAT.drawingToDisplayArea = (parameter >> 10) & 1;
    GPUSTAT.texturePageYBase2 = (parameter >> 11) & 1;
}

void festation::PsxGpu::parseCommandGP1(uint32_t commandWord)
{
    uint8_t command = commandWord >> 24;
    uint32_t parameter = commandWord & 0x00FFFFFF;

    switch (command)
    {
    case Gpu1Commands::ResetGpu:
        processResetGpuCmd();
        break;
    case Gpu1Commands::ResetCommandBuffer:
        processResetCommandBufferCmd();
        break;
    case Gpu1Commands::AcknowledgeGpuInterrupt:
        processAckGpuIntCmd();
        break;
    case Gpu1Commands::DisplayEnable:
        processDisplayEnableCmd(parameter);
        break;
    case Gpu1Commands::DmaDirectionDataRequest:
        processDmaDirectionDataRequestCmd(parameter);
        break;
    case Gpu1Commands::StartDisplayArea:
        processStartDisplayAreaCmd(parameter);
        break;
    case Gpu1Commands::HorizontalDisplayRange:
        processHorizontalDisplayRangeCmd(parameter);
        break;
    case Gpu1Commands::VerticalDisplayRange:
        processVerticalDisplayRangeCmd(parameter);
        break;
    case Gpu1Commands::DisplayMode:
        processDisplayModeCmd(parameter);
        break;
    case Gpu1Commands::SetVramSize:
        processSetVramSizeCmd(parameter);
        break;
    case Gpu1Commands::ReadGpuInternalRegister:
        processReadGpuInternalRegCmd(parameter);
        break;
    default:
        LOG_DEBUG("Unimplemented GP1 GPU command ({:x}h)", command);
        break;
    }
}

void festation::PsxGpu::processResetGpuCmd()
{
    GPUSTAT.raw = 0x14802000;
    GPUSTAT.readyToSendVRAMtoCPU = 1; // TODO: TEMP
}

void festation::PsxGpu::processResetCommandBufferCmd()
{
    std::memset(m_commandsFIFO.data(), 0, m_commandsFIFO.size());
    m_currentCmdArg = 0;
}

void festation::PsxGpu::processAckGpuIntCmd()
{
    GPUSTAT.interruptRequest = 0;
}

void festation::PsxGpu::processDisplayEnableCmd(uint32_t parameter)
{
}

void festation::PsxGpu::processDmaDirectionDataRequestCmd(uint32_t parameter)
{
}

void festation::PsxGpu::processStartDisplayAreaCmd(uint32_t parameter)
{
}

void festation::PsxGpu::processHorizontalDisplayRangeCmd(uint32_t parameter)
{
}

void festation::PsxGpu::processVerticalDisplayRangeCmd(uint32_t parameter)
{
}

void festation::PsxGpu::processDisplayModeCmd(uint32_t parameter)
{
}

void festation::PsxGpu::processSetVramSizeCmd(uint32_t parameter)
{
}

void festation::PsxGpu::processReadGpuInternalRegCmd(uint32_t parameter)
{
}
