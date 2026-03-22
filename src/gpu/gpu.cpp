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
    updateRenderProjection();
    m_renderer.uploadVramToGpu((const uint8_t*)m_vram.data(), { 0, 0 }, { VRAM_WIDTH, VRAM_HEIGHT });
}

festation::PsxGpu::~PsxGpu()
{
}

auto festation::PsxGpu::read32(uint32_t address) -> uint32_t
{
    switch(address) {
    case 0x1F801810:
        if (m_commandState == GpuCommandsState::ProcessingVramCpuBlitCmd 
            && m_vramCpuBlitCmdInfo.cmdState == BlittingCommandsState::ReceivingData) {               
            const auto lengthX = m_vramCpuBlitCmdInfo.size2D.x;
            const auto lengthY = m_vramCpuBlitCmdInfo.size2D.y;
            const auto y = m_vramCpuBlitCmdInfo.srcCoord.y;
            const auto x = m_vramCpuBlitCmdInfo.srcCoord.x;
            const auto offsetX = (m_vramCpuBlitCmdInfo.size - m_remainingCmdArg) % lengthX;
            const auto offsetY = (m_vramCpuBlitCmdInfo.size - m_remainingCmdArg) / lengthY;
            const auto destY = y + offsetY;
            const auto destX = x + offsetX;
            size_t offsetDst = (destY * VRAM_WIDTH) + destX;

            m_gp0ReadValue = m_vram[offsetDst];
            m_gp0ReadValue |= m_vram[offsetDst + 1] << 16;
            m_remainingCmdArg -= 2;

            if (m_remainingCmdArg == 0) {
                processResetCommandBufferCmd();
            }
        }
        else {
            m_gp0ReadValue = GPUREAD.raw;
        }

        return m_gp0ReadValue;
    case 0x1F801814:
        return GPUSTAT.raw;
    default:
        std::unreachable();
    }
}

auto festation::PsxGpu::write32(uint32_t address, uint32_t value) -> void
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
        case GpuCommandsState::ProcessingQuickRectFillCmdParams:
            processGP0QuickRectFillCmd(value);
            break;
        case GpuCommandsState::ProcessingPolygonCmdParams:
            processGP0PolygonCmd(value);
            break;
        case GpuCommandsState::ProcessingLineCmdParams:
            processGP0LineCmd(value);
            break;
        case GpuCommandsState::ProcessingVramVramBlitCmdParams:
            processGP0VramVramBlitCmd(value);
            break;
        case GpuCommandsState::ProcessingCpuVramBlitCmd:
            processGP0CpuVramBlitCmd(value);
            break;
        case GpuCommandsState::ProcessingVramCpuBlitCmd:
            processGP0VramCpuBlitCmd(value);
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

auto festation::PsxGpu::renderFrame() -> void
{
    m_renderer.renderFrame();
}

auto festation::PsxGpu::parseCommandGP0(uint32_t commandWord) -> void
{
    LOG_DEBUG("Getting GP0 command ({:08X}h)", commandWord);

    uint8_t command = commandWord >> 29;
    uint8_t fullCmd = commandWord >> 24;

    switch(command)
    {
    case Gpu0Commands::PolygonPrimitive:
        m_polyData.isGouraudShading = (commandWord >> 28) & 0x1;
        m_polyData.verticesCount = ((commandWord >> 27) & 0x1) ? 4 : 3;
        m_polyData.isTextured = (commandWord >> 26) & 0x1;
        m_polyData.isSemiTransparent = (commandWord >> 25) & 0x1;
        m_polyData.isRawTexture = (commandWord >> 24) & 0x1;

        m_remainingCmdArg =  m_polyData.verticesCount; // At least 3/4 vertices words (flat shading and not textured)

        if (m_polyData.isGouraudShading)
            m_remainingCmdArg += (m_polyData.verticesCount - 1); // 1st color already provided

        if (m_polyData.isTextured)
            m_remainingCmdArg += m_polyData.verticesCount;

        m_commandsFIFO[m_currentCmdParam++] = commandWord;
        m_commandState = GpuCommandsState::ProcessingPolygonCmdParams;
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
        // m_remainingCmdArg = 3;
        // m_commandsFIFO[m_currentCmdParam++] = commandWord;
        // m_commandState = GpuCommandsState::ProcessingVramVramBlitCmdParams;
        break;
    case Gpu0Commands::CpuToVramBlit:
        m_remainingCmdArg = 2;
        m_cpuVramBlitCmdInfo.cmdState = BlittingCommandsState::ReceivingParams;
        m_commandsFIFO[m_currentCmdParam++] = commandWord;
        m_commandState = GpuCommandsState::ProcessingCpuVramBlitCmd;
        break;
    case Gpu0Commands::VramToCpuBlit:
        // m_remainingCmdArg = 2;
        // m_vramCpuBlitCmdInfo.cmdState = BlittingCommandsState::ReceivingParams;
        // m_commandsFIFO[m_currentCmdParam++] = commandWord;
        // m_commandState = GpuCommandsState::ProcessingVramCpuBlitCmd;
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
            m_remainingCmdArg = 2;
            m_commandsFIFO[m_currentCmdParam++] = commandWord;
            m_commandState = GpuCommandsState::ProcessingQuickRectFillCmdParams;
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
            LOG_DEBUG("Unimplemented GP0 Misc/Env GPU command ({:X}h)", fullCmd);
            break;
        }
        break;
    default:
        LOG_DEBUG("Unimplemented GP0 GPU command (0b{:b})", command);
        break;
    }
}

auto festation::PsxGpu::processGP0PolygonCmd(uint32_t parameter) -> void
{
    m_commandsFIFO[m_currentCmdParam++] = parameter;
    m_remainingCmdArg--;

    if (m_remainingCmdArg == 0) {
        size_t colorParamOffset, vertexParamOffset, clutPageUVParamOffset;

        for (size_t vertexId = 0; vertexId < m_polyData.verticesCount; vertexId++) {

            if (m_polyData.isGouraudShading && m_polyData.isTextured) {
                colorParamOffset = vertexId * 3;
                vertexParamOffset = vertexId * 3 + 1;
                clutPageUVParamOffset = vertexId * 3 + 2;
            }
            else if (m_polyData.isGouraudShading && !m_polyData.isTextured) {
                colorParamOffset = vertexId * 2;
                vertexParamOffset = vertexId * 2 + 1;
            }
            else if (!m_polyData.isGouraudShading && m_polyData.isTextured) {
                colorParamOffset = 0;
                vertexParamOffset = vertexId * 2 + 1;
                clutPageUVParamOffset = vertexId * 2 + 2;
            }
            else {
                colorParamOffset = 0;
                vertexParamOffset = vertexId + 1;
            }

            const auto& colorParam = m_commandsFIFO[colorParamOffset];
            auto&  color = m_polyData.colors[vertexId];
            color.a = 1.0f;
            color.r = colorParam & 0xFF;
            color.g = (colorParam >> 8) & 0xFF;
            color.b = (colorParam >> 16) & 0xFF;

            const auto& vertexParam = m_commandsFIFO[vertexParamOffset];
            auto& vertex = m_polyData.vertices[vertexId];
            vertex.x = int16_t(vertexParam & 0x7FF) + m_drawingAreaInfo.offset.x;
            vertex.y = int16_t((vertexParam >> 16) & 0x7FF) + m_drawingAreaInfo.offset.y;

            if (m_polyData.isTextured) {
                const auto& clutPageUVParam = m_commandsFIFO[clutPageUVParamOffset];
                auto& uv = m_polyData.uvs[vertexId];
                uv.x = clutPageUVParam & 0xFFu;
                uv.y = (clutPageUVParam >> 8) & 0xFFu;

                if (vertexId == 0) {
                    auto& clut = m_polyData.clut;
                    clut.x = (clutPageUVParam >> 16) & 0x3Fu;
                    clut.y = (clutPageUVParam >> 22) & 0x1FFu;
                }

                if (vertexId == 1) {
                    auto& page = m_polyData.page;
                    page.x = (clutPageUVParam >> 16) & 0x3Fu;
                    page.y = (clutPageUVParam >> 22) & 0x1FFu;
                }
            }
        }
        
        m_renderer.drawPolygon(m_polyData);

        processResetCommandBufferCmd();
    }
}

auto festation::PsxGpu::processGP0LineCmd(uint32_t parameter) -> void
{
}

auto festation::PsxGpu::processGP0RectangleCmd(uint32_t parameter) -> void
{
    m_commandsFIFO[m_currentCmdParam++] = parameter;
    m_remainingCmdArg--;

    if (m_remainingCmdArg == 0) {
        const auto& color = m_commandsFIFO[RECT_COLOR_PARAM_POS];
        m_rectData.color.a = 1.0f;
        m_rectData.color.r = color & 0xFFu;
        m_rectData.color.g = (color >> 8) & 0xFFu;
        m_rectData.color.b = (color >> 16) & 0xFFu;

        const auto& vertex = m_commandsFIFO[RECT_VERTEX_PARAM_POS];
        m_rectData.vertex1.x = int16_t(vertex & 0x7FFu) + m_drawingAreaInfo.offset.x;
        m_rectData.vertex1.y = int16_t((vertex >> 16) & 0x7FFu) + m_drawingAreaInfo.offset.y;

        const auto& clutUV = m_commandsFIFO[RECT_UV_PARAM_POS];
        m_rectData.clutUV.uv.x = clutUV & 0xFFu;
        m_rectData.clutUV.uv.y = (clutUV >> 8) & 0xFFu;
        m_rectData.clutUV.clut.x = (clutUV >> 16) & 0x3Fu;
        m_rectData.clutUV.clut.y = (clutUV >> 22) & 0x1FFu;

        const auto& dimensions = m_commandsFIFO[RECT_SIZE_PARAM_POS];

        switch(m_rectData.sizeType)
        {
        case RectanglePrimitiveData::Variable:
            m_rectData.size.x = dimensions & 0x7FFu;
            m_rectData.size.y = (dimensions >> 16) & 0x7FFu;
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
        
        m_renderer.drawRectangle(m_rectData);

        processResetCommandBufferCmd();
    }
}

auto festation::PsxGpu::processGP0VramVramBlitCmd(uint32_t parameter) -> void
{
    m_commandsFIFO[m_currentCmdParam++] = parameter;
    m_remainingCmdArg--;

    if (m_remainingCmdArg == 0) {
        const auto& srcCoordParam = m_commandsFIFO[1];
        glm::u16vec2 srcCoord;
        srcCoord.x = srcCoordParam & 0x3FFu;
        srcCoord.y = (srcCoordParam >> 16) & 0x1FFu;

        const auto& dstCoordParam = m_commandsFIFO[2];
        glm::u16vec2 dstCoord;
        dstCoord.x = dstCoordParam & 0x3FFu;
        dstCoord.y = (dstCoordParam >> 16) & 0x1FFu;

        const auto& sizeCoordParam = m_commandsFIFO[3];
        glm::u16vec2 size;
        size.x = ((sizeCoordParam - 1) & 0x3FFu) + 1;
        size.y = (((sizeCoordParam >> 16) - 1) & 0x1FFu) + 1;

        for (size_t line = 0; line < size.y; line++) {
            size_t offsetDst = (dstCoord.y * size.x) + dstCoord.x;
            size_t offsetSrc = (srcCoord.y * size.x) + srcCoord.x;
            std::memcpy(m_vram.data() + offsetDst, m_vram.data() + offsetSrc, size.x);
        }

        processResetCommandBufferCmd();
    }
}

auto festation::PsxGpu::processGP0CpuVramBlitCmd(uint32_t parameter) -> void
{
    switch (m_cpuVramBlitCmdInfo.cmdState)
    {
    case BlittingCommandsState::ReceivingParams:
        m_commandsFIFO[m_currentCmdParam++] = parameter;
        m_remainingCmdArg--;

        if (m_remainingCmdArg == 0) {
            const auto& dstCoordParam = m_commandsFIFO[1];
            m_cpuVramBlitCmdInfo.dstCoord.x = dstCoordParam & 0x3FFu;
            m_cpuVramBlitCmdInfo.dstCoord.y = (dstCoordParam >> 16) & 0x1FFu;

            /** @todo: Account for odd sizes */
            const auto& sizeCoordParam = m_commandsFIFO[2];
            m_cpuVramBlitCmdInfo.size2D.x = ((sizeCoordParam - 1) & 0x3FFu) + 1;
            m_cpuVramBlitCmdInfo.size2D.y = (((sizeCoordParam >> 16) - 1) & 0x1FFu) + 1;

            m_cpuVramBlitCmdInfo.size = m_cpuVramBlitCmdInfo.size2D.x * m_cpuVramBlitCmdInfo.size2D.y;
            m_cpuVramBlitCmdInfo.blitData.clear();
            m_cpuVramBlitCmdInfo.blitData.resize(m_cpuVramBlitCmdInfo.size);
            m_cpuVramBlitCmdInfo.totalWords = (m_cpuVramBlitCmdInfo.size + 1) / 2;
            m_cpuVramBlitCmdInfo.currentWord = 0;
            m_cpuVramBlitCmdInfo.cmdState = BlittingCommandsState::ReceivingData;
        }
        break;
    case BlittingCommandsState::ReceivingData:
        {
            const auto lengthX = m_cpuVramBlitCmdInfo.size2D.x;
            const auto lengthY = m_cpuVramBlitCmdInfo.size2D.y;
            const auto coordY = m_cpuVramBlitCmdInfo.dstCoord.y;
            const auto coordX = m_cpuVramBlitCmdInfo.dstCoord.x;
            const auto offsetX = (m_cpuVramBlitCmdInfo.currentWord * 2) % lengthX;
            const auto offsetY = (m_cpuVramBlitCmdInfo.currentWord * 2) / lengthX;
            const auto destY = coordY + offsetY;
            const auto destX = coordX + offsetX;
            size_t offsetDst = (destY * VRAM_WIDTH) + destX;
            size_t offsetBlit = (offsetY * lengthX) + offsetX;

            m_cpuVramBlitCmdInfo.blitData[offsetBlit] = parameter & 0xFFFF;
            m_cpuVramBlitCmdInfo.blitData[offsetBlit + 1] = (parameter >> 16) & 0xFFFF;
            m_vram[offsetDst] = m_cpuVramBlitCmdInfo.blitData[offsetBlit];
            m_vram[offsetDst + 1] = m_cpuVramBlitCmdInfo.blitData[offsetBlit + 1];
            m_cpuVramBlitCmdInfo.currentWord++;

            if (m_cpuVramBlitCmdInfo.currentWord == m_cpuVramBlitCmdInfo.totalWords) {
                uint8_t* data = reinterpret_cast<uint8_t *>(m_cpuVramBlitCmdInfo.blitData.data());
                m_renderer.uploadVramToGpu(data, { coordX, VRAM_HEIGHT - lengthY - coordY }, { lengthX, lengthY });

                processResetCommandBufferCmd();
            }
        }
        break;
    default:
        std::unreachable();
    }
}

auto festation::PsxGpu::processGP0VramCpuBlitCmd(uint32_t parameter) -> void
{
    switch (m_vramCpuBlitCmdInfo.cmdState)
    {
    case BlittingCommandsState::ReceivingParams:
        m_commandsFIFO[m_currentCmdParam++] = parameter;
        m_remainingCmdArg--;

        if (m_remainingCmdArg == 0) {
            const auto& srcCoordParam = m_commandsFIFO[1];
            m_vramCpuBlitCmdInfo.srcCoord.x = srcCoordParam & 0x3FFu;
            m_vramCpuBlitCmdInfo.srcCoord.y = (srcCoordParam >> 16) & 0x1FFu;

            /** @todo: Account for odd sizes */
            const auto& sizeCoordParam = m_commandsFIFO[2];
            m_vramCpuBlitCmdInfo.size2D.x = ((sizeCoordParam - 1) & 0x3FFu) + 1;
            m_vramCpuBlitCmdInfo.size2D.y = (((sizeCoordParam >> 16) - 1) & 0x1FFu) + 1;

            m_vramCpuBlitCmdInfo.size = m_vramCpuBlitCmdInfo.size2D.x * m_vramCpuBlitCmdInfo.size2D.y;
            m_remainingCmdArg = m_vramCpuBlitCmdInfo.size;
            m_vramCpuBlitCmdInfo.cmdState = BlittingCommandsState::ReceivingData;
        }
        break;
    case BlittingCommandsState::ReceivingData:
        break;
    default:
        std::unreachable();
    }
}

auto festation::PsxGpu::processGP0ClearCacheCmd() -> void
{
}

auto festation::PsxGpu::processGP0QuickRectFillCmd(uint32_t parameter) -> void
{
    m_commandsFIFO[m_currentCmdParam++] = parameter;
    m_remainingCmdArg--;

    if (m_remainingCmdArg == 0) {
        const auto& colorParam = m_commandsFIFO[0];
        glm::vec4 color;
        
        color.a = 1.0f;
        color.r = (colorParam & 0xFF) / 255.0f;
        color.g = ((colorParam >> 8) & 0xFFu) / 255.0f;
        color.b = ((colorParam >> 16) & 0xFFu) / 255.0f;

        const auto& topLeftCoordsParam = m_commandsFIFO[1];
        glm::u16vec2 topLeftCoords;

        topLeftCoords.x = topLeftCoordsParam & 0x3F0u;
        topLeftCoords.y = (topLeftCoordsParam >> 16) & 0x1FFu;

        const auto& sizeParam = m_commandsFIFO[2];
        glm::u16vec2 size;

        size.x = ((sizeParam & 0x3FFu) + 0x0Fu) & ~0x0Fu;
        size.y = (sizeParam >> 16) & 0x1FFu;

        if (size.x != 0 && size.y != 0) {
            /** @todo: Create VRAM framebuffer on device side and fill it as stated by command parameters */
        }

        processResetCommandBufferCmd();
    }
}

auto festation::PsxGpu::processGP0InterruptRequestCmd() -> void
{
}

auto festation::PsxGpu::processGP0DrawModeCmd(uint32_t parameter) -> void
{
    GPUSTAT.raw = (GPUSTAT.raw & 0xFFFFFC00u) | (parameter & 0x3FFu);
    GPUSTAT.drawingToDisplayArea = (parameter >> 10) & 1;
    GPUSTAT.texturePageYBase2 = (parameter >> 11) & 1;
}

auto festation::PsxGpu::processGP0TextureWindowCmd(uint32_t parameter) -> void
{ 
}

auto festation::PsxGpu::processGP0SetDrawingAreaX1Y1Cmd(uint32_t parameter) -> void
{
    m_renderer.renderFrame();

    m_drawingAreaInfo.topLeft.x = parameter & 0x3FFu;
    m_drawingAreaInfo.topLeft.y = (parameter >> 10) & 0x1FFu;

    uint16_t flippedY = VRAM_HEIGHT - (m_drawingAreaInfo.topLeft.y + m_drawingAreaInfo.bottomRight.y);
    m_renderer.setClipRegion({ m_drawingAreaInfo.topLeft.x, flippedY }, m_drawingAreaInfo.bottomRight);

    // updateRenderProjection();
    // m_renderer.setViewport(m_drawingAreaInfo.topLeft, m_drawingAreaInfo.bottomRight);
}

auto festation::PsxGpu::processGP0SetDrawingAreaX2Y2Cmd(uint32_t parameter) -> void
{
    m_renderer.renderFrame();

    m_drawingAreaInfo.bottomRight.x = parameter & 0x3FFu;
    m_drawingAreaInfo.bottomRight.y = (parameter >> 10) & 0x1FFu; 

    uint16_t flippedY = VRAM_HEIGHT - (m_drawingAreaInfo.topLeft.y + m_drawingAreaInfo.bottomRight.y);
    m_renderer.setClipRegion({ m_drawingAreaInfo.topLeft.x, flippedY }, m_drawingAreaInfo.bottomRight);

    // updateRenderProjection();
    // m_renderer.setViewport(m_drawingAreaInfo.topLeft, m_drawingAreaInfo.bottomRight);
}

auto festation::PsxGpu::processGP0SetDrawingOffsetCmd(uint32_t parameter) -> void
{
    m_drawingAreaInfo.offset.x = parameter & 0x3FFu;
    m_drawingAreaInfo.offset.y = (parameter >> 10) & 0x3FFu; 
}

auto festation::PsxGpu::processGP0MaskBitSettingCmd(uint32_t parameter) -> void
{
    GPUSTAT.setMaskbitWhenDrawing = parameter & 1;
    GPUSTAT.drawPixels = (parameter >> 1) & 1;
}

auto festation::PsxGpu::parseCommandGP1(uint32_t commandWord) -> void
{
    uint8_t command = commandWord >> 24;
    uint32_t parameter = commandWord & 0x00FFFFFFu;

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

auto festation::PsxGpu::processResetGpuCmd() -> void
{
    GPUSTAT.raw = 0x14802000;
    GPUSTAT.readyToReceiveCmdWord = 1;
    GPUSTAT.readyToSendVRAMtoCPU = 1; // TODO: TEMP
    GPUSTAT.readyToReceiveDMABlock = 1;
    m_commandState = GpuCommandsState::WaitingForCommand;
}

auto festation::PsxGpu::processResetCommandBufferCmd() -> void
{
    std::memset(m_commandsFIFO.data(), 0, m_commandsFIFO.size() * sizeof(uint32_t));
    m_currentCmdParam = 0;
    m_remainingCmdArg = 0;
    m_commandState = GpuCommandsState::WaitingForCommand;
}

auto festation::PsxGpu::processAckGpuIntCmd() -> void
{
    GPUSTAT.interruptRequest = 0;
}

auto festation::PsxGpu::processDisplayEnableCmd(uint32_t parameter) -> void
{
}

auto festation::PsxGpu::processDmaDirectionDataRequestCmd(uint32_t parameter) -> void
{
    GPUSTAT.dmaDirection = parameter & 3;

    switch (GPUSTAT.dmaDirection)
    {
    case 0:
        GPUSTAT.dmaDataRequest = 0;
        break;
    case 1:
        GPUSTAT.dmaDataRequest = 1;
        break;
    case 2:
        GPUSTAT.dmaDataRequest = GPUSTAT.readyToSendVRAMtoCPU;
        break;
    case 3:
        GPUSTAT.dmaDataRequest = GPUSTAT.readyToReceiveDMABlock;
        break;
    default:
        std::unreachable();
    }
}

auto festation::PsxGpu::processStartDisplayAreaCmd(uint32_t parameter) -> void
{
}

auto festation::PsxGpu::processHorizontalDisplayRangeCmd(uint32_t parameter) -> void
{
}

auto festation::PsxGpu::processVerticalDisplayRangeCmd(uint32_t parameter) -> void
{
}

auto festation::PsxGpu::processDisplayModeCmd(uint32_t parameter) -> void
{
}

auto festation::PsxGpu::processSetVramSizeCmd(uint32_t parameter) -> void
{
}

auto festation::PsxGpu::processReadGpuInternalRegCmd(uint32_t parameter) -> void
{
}

auto festation::PsxGpu::updateRenderProjection() -> void
{
    // float width = m_drawingAreaInfo.bottomRight.x - m_drawingAreaInfo.topLeft.x + 1;
    // float height = m_drawingAreaInfo.bottomRight.y - m_drawingAreaInfo.topLeft.y + 1;
    glm::mat4 projection = glm::ortho(0.0f, (float)VRAM_WIDTH, (float)VRAM_HEIGHT, 0.0f);
    m_renderer.setProjection(projection);
}
