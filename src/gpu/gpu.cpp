#include "gpu.hpp"
#include "gpu_commands.h"
#include "utils/logger.hpp"

#include <utility>

#include <glm/gtc/matrix_transform.hpp>

festation::PsxGpu::PsxGpu()
    : GPUREAD({}), GPUSTAT({}), m_commandState(GpuCommandsState::WaitingForCommand),
        m_remainingCmdArg(1), m_currentCmdParam(0), m_commandsFIFO({}), m_vram(VRAM_WIDTH * VRAM_HEIGHT)
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
        case GpuCommandsState::ProcessingRectCmdParams:
            processGP0RectangleCmd(value);
            break;
        default:
            std::unreachable();
        }
        break;
    case 0x1F801814:
        parseCommandGP1(value);
        break;
    default:
        std::unreachable();
    }
}

void festation::PsxGpu::renderFrame()
{
    m_renderer.renderFrame();
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
        m_rectData.sizeType = RectanglePrimitiveData::RectSizeType((commandWord >> 27) & 0x3);
        m_rectData.isTextured = (commandWord >> 26) & 0x1;
        m_rectData.isSemiTransparent = (commandWord >> 25) & 0x1;
        m_rectData.isRawTexture = (commandWord >> 24) & 0x1;

        m_remainingCmdArg = 1;

        if (m_rectData.isTextured) {
            m_remainingCmdArg++;
        }

        if (m_rectData.sizeType == RectanglePrimitiveData::Variable) {
            m_remainingCmdArg++;
        }

        m_commandsFIFO[m_currentCmdParam++] = commandWord;
        m_commandState = GpuCommandsState::ProcessingRectCmdParams;
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
        case Gpu0Commands::NOP:
            break;
        case Gpu0Commands::ClearCache:
            processGP0ClearCacheCmd();
            break;
        case Gpu0Commands::QuickRectangleFill:
            processGP0QuickRectFillCmd(commandWord);
            break;
        case Gpu0Commands::InterruptRequest:
            processGP0InterruptRequestCmd();
            break;
        case Gpu0Commands::DrawMode:
            processGP0DrawModeCmd(commandWord);
            break;
        case Gpu0Commands::TextureWindow:
            processGP0TextureWindowCmd(commandWord);
            break;
        case Gpu0Commands::SetDrawingAreaX1Y1:
            processGP0SetDrawingAreaX1Y1Cmd(commandWord);
            break;
        case Gpu0Commands::SetDrawingAreaX2Y2:
            processGP0SetDrawingAreaX2Y2Cmd(commandWord);
            break;
        case Gpu0Commands::SetDrawingOffset:
            processGP0SetDrawingOffsetCmd(commandWord);
            break;
        case Gpu0Commands::MaskBitSetting:
            processGP0MaskBitSettingCmd(commandWord);
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
    m_commandsFIFO[m_currentCmdParam++] = parameter;
    m_remainingCmdArg--;

    if (m_remainingCmdArg == 0) {
        const auto& color = m_commandsFIFO[COLOR_PARAM_POS];
        m_rectData.color.a = 1.0f;
        m_rectData.color.r = color & 0xFF;
        m_rectData.color.g = (color >> 8) & 0xFF;
        m_rectData.color.b = (color >> 16) & 0xFF;

        const auto& vertex = m_commandsFIFO[VERTEX_PARAM_POS];
        m_rectData.vertex1.x = int16_t(vertex & 0x7FF) + m_drawingAreaInfo.offset.x;
        m_rectData.vertex1.y = int16_t((vertex >> 16) & 0x7FF) + m_drawingAreaInfo.offset.y;

        const auto& clutUV = m_commandsFIFO[UV_PARAM_POS];
        m_rectData.clutUV.uv.x = clutUV & 0x3F;
        m_rectData.clutUV.uv.y = (clutUV >> 6) & 0x1FF;
        m_rectData.clutUV.clut = glm::i16vec2((clutUV >> 16) & 0xFFFF);

        const auto& dimensions = m_commandsFIFO[RECT_SIZE_PARAM_POS];

        switch(m_rectData.sizeType)
        {
        case RectanglePrimitiveData::Variable:
            m_rectData.size.x = dimensions & 0x7FF;
            m_rectData.size.y = (dimensions >> 16) & 0x7FF;
            break;
        case RectanglePrimitiveData::SinglePixel:
            m_rectData.size.x = 1u;
            m_rectData.size.y = 1u;
            break;
        case RectanglePrimitiveData::Sprite8x8:
            m_rectData.size.x = 8u;
            m_rectData.size.y = 8u;
            break;
        case RectanglePrimitiveData::Sprite16x16:
            m_rectData.size.x = 16u;
            m_rectData.size.y = 16u;
            break;
        default:
            std::unreachable();
        }

        m_renderer.setViewport(m_drawingAreaInfo.topLeft, m_drawingAreaInfo.bottomRight - m_drawingAreaInfo.topLeft);
        
        m_renderer.drawRectangle(m_rectData);

        m_currentCmdParam = 0;
        m_commandState = GpuCommandsState::WaitingForCommand;
    }
}

void festation::PsxGpu::processGP0ClearCacheCmd()
{
}

void festation::PsxGpu::processGP0QuickRectFillCmd(uint32_t parameter)
{
}

void festation::PsxGpu::processGP0InterruptRequestCmd()
{
}

void festation::PsxGpu::processGP0DrawModeCmd(uint32_t parameter)
{
    GPUSTAT.raw = (GPUSTAT.raw & 0xFFFFFC00) | (parameter & 0x3FF);
    GPUSTAT.drawingToDisplayArea = (parameter >> 10) & 1;
    GPUSTAT.texturePageYBase2 = (parameter >> 11) & 1;
}

void festation::PsxGpu::processGP0TextureWindowCmd(uint32_t parameter)
{ 
}

void festation::PsxGpu::processGP0SetDrawingAreaX1Y1Cmd(uint32_t parameter)
{
    m_drawingAreaInfo.topLeft.x = parameter & 0x3FF;
    m_drawingAreaInfo.topLeft.y = (parameter >> 10) & 0x1FF;

    updateRenderProjection();
}

void festation::PsxGpu::processGP0SetDrawingAreaX2Y2Cmd(uint32_t parameter)
{
    m_drawingAreaInfo.bottomRight.x = parameter & 0x3FF;
    m_drawingAreaInfo.bottomRight.y = (parameter >> 10) & 0x1FF; 

    updateRenderProjection();
}

void festation::PsxGpu::processGP0SetDrawingOffsetCmd(uint32_t parameter)
{
    m_drawingAreaInfo.offset.x = parameter & 0x3FF;
    m_drawingAreaInfo.offset.y = (parameter >> 10) & 0x3FF; 
}

void festation::PsxGpu::processGP0MaskBitSettingCmd(uint32_t parameter)
{
    GPUSTAT.setMaskbitWhenDrawing = parameter & 1;
    GPUSTAT.drawPixels = (parameter >> 1) & 1;
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
    m_remainingCmdArg = 0;
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

void festation::PsxGpu::updateRenderProjection()
{
    float width = m_drawingAreaInfo.bottomRight.x - m_drawingAreaInfo.topLeft.x + 1;
    float height = m_drawingAreaInfo.bottomRight.y - m_drawingAreaInfo.topLeft.y + 1;
    glm::mat4 projection = glm::ortho(0.0f, width, height, 0.0f);
    m_renderer.setProjection(projection);
}
