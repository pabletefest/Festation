#pragma once

#include "gpu_commands.h"
#include "primitives_data.hpp"
#include "renderer/renderer.hpp"

#include <cstdint>
#include <array>
#include <vector>

#include <glm/vec2.hpp>

namespace festation {
    static constexpr size_t VRAM_WIDTH = 1024;
    static constexpr size_t VRAM_HEIGHT = 512;

    class PsxGpu {
    public:
        PsxGpu();
        ~PsxGpu();

        auto read32(uint32_t address) -> uint32_t;
        auto write32(uint32_t address, uint32_t value) -> void;

        auto renderFrame() -> void;

    private:
        auto parseCommandGP0(uint32_t commandWord) -> void;
        auto processGP0PolygonCmd(uint32_t parameter) -> void;
        auto processGP0LineCmd(uint32_t parameter) -> void;
        auto processGP0RectangleCmd(uint32_t parameter) -> void;
        auto processGP0VramVramBlitCmd(uint32_t parameter) -> void;
        auto processGP0CpuVramBlitCmd(uint32_t parameter) -> void;
        auto processGP0VramCpuBlitCmd(uint32_t parameter) -> void;
        auto processGP0ClearCacheCmd() -> void;
        auto processGP0QuickRectFillCmd(uint32_t parameter) -> void;
        auto processGP0InterruptRequestCmd() -> void;
        auto processGP0DrawModeCmd(uint32_t parameter) -> void;
        auto processGP0TextureWindowCmd(uint32_t parameter) -> void;
        auto processGP0SetDrawingAreaX1Y1Cmd(uint32_t parameter) -> void;
        auto processGP0SetDrawingAreaX2Y2Cmd(uint32_t parameter) -> void;
        auto processGP0SetDrawingOffsetCmd(uint32_t parameter) -> void;
        auto processGP0MaskBitSettingCmd(uint32_t parameter) -> void;
        
        auto parseCommandGP1(uint32_t commandWord) -> void;
        auto processResetGpuCmd() -> void;
        auto processResetCommandBufferCmd() -> void;
        auto processAckGpuIntCmd() -> void;
        auto processDisplayEnableCmd(uint32_t parameter) -> void;
        auto processDmaDirectionDataRequestCmd(uint32_t parameter) -> void;
        auto processStartDisplayAreaCmd(uint32_t parameter) -> void;
        auto processHorizontalDisplayRangeCmd(uint32_t parameter) -> void;
        auto processVerticalDisplayRangeCmd(uint32_t parameter) -> void;
        auto processDisplayModeCmd(uint32_t parameter) -> void;
        auto processSetVramSizeCmd(uint32_t parameter) -> void;
        auto processReadGpuInternalRegCmd(uint32_t parameter) -> void;

        auto updateRenderProjection() -> void;

    private:
        union {
            struct {

            };

            uint32_t raw;
        } GPUREAD{};

        union {
            struct {
                uint32_t texturePageXBase : 4;
                uint32_t texturePageYBase1 : 1;
                uint32_t semiTransparency : 2;
                TexturePageColorsDepth texturePageColors : 2;
                uint32_t dither24bitTo15bit: 1;
                uint32_t drawingToDisplayArea : 1;
                uint32_t setMaskbitWhenDrawing : 1;
                uint32_t drawPixels : 1;
                uint32_t interlaceField : 1;
                uint32_t flipScreenHorizontally : 1;
                uint32_t texturePageYBase2 : 1;
                uint32_t horizontalResolution2 : 1;
                uint32_t horizontalResolution1 : 2;
                uint32_t verticalResolution : 1;
                uint32_t videoMode : 1;
                uint32_t displayAreaColorDepth : 1;
                uint32_t verticalInterlace : 1;
                uint32_t displayEnable : 1;
                uint32_t interruptRequest : 1;
                uint32_t dmaDataRequest : 1;
                uint32_t readyToReceiveCmdWord : 1;
                uint32_t readyToSendVRAMtoCPU : 1;
                uint32_t readyToReceiveDMABlock : 1;
                uint32_t dmaDirection : 2;
                uint32_t drawingEvenOddLinesInInterlaceMode : 1;
            };
            
            uint32_t raw;
        } GPUSTAT{};

        DrawingAreaInfo m_drawingAreaInfo{};

        struct CpuToVramBlitCmdInfo {
            BlittingCommandsState cmdState;
            glm::u16vec2 dstCoord;
            glm::u16vec2 size2D;
            size_t size;
            size_t totalWords;
            size_t currentWord;
            std::vector<uint16_t> blitData;
        } m_cpuVramBlitCmdInfo;

        struct VramToCpuBlitCmdInfo {
            BlittingCommandsState cmdState;
            glm::u16vec2 srcCoord;
            glm::u16vec2 size2D;
            size_t size;
        } m_vramCpuBlitCmdInfo;

        uint32_t m_gp0ReadValue;

        RectanglePrimitiveData m_rectData{};
        PolygonPrimitiveData m_polyData{};

        GpuCommandsState m_commandState{};
        size_t m_remainingCmdArg{};
        size_t m_currentCmdParam{};
        static constexpr size_t MAX_COMMANDS_BUFFER_SIZE = 16;
        std::array<uint32_t, MAX_COMMANDS_BUFFER_SIZE> m_commandsFIFO{};

        std::vector<uint16_t> m_vram{};

        Renderer m_renderer;
    };
};
