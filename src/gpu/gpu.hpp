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

        uint32_t read32(uint32_t address);
        void write32(uint32_t address, uint32_t value);

        void renderFrame();

    private:
        void parseCommandGP0(uint32_t commandWord);
        void processGP0RectangleCmd(uint32_t parameter);
        void processGP0ClearCacheCmd();
        void processGP0QuickRectFillCmd(uint32_t parameter);
        void processGP0InterruptRequestCmd();
        void processGP0DrawModeCmd(uint32_t parameter);
        void processGP0TextureWindowCmd(uint32_t parameter);
        void processGP0SetDrawingAreaX1Y1Cmd(uint32_t parameter);
        void processGP0SetDrawingAreaX2Y2Cmd(uint32_t parameter);
        void processGP0SetDrawingOffsetCmd(uint32_t parameter);
        void processGP0MaskBitSettingCmd(uint32_t parameter);
        
        void parseCommandGP1(uint32_t commandWord);
        void processResetGpuCmd();
        void processResetCommandBufferCmd();
        void processAckGpuIntCmd();
        void processDisplayEnableCmd(uint32_t parameter);
        void processDmaDirectionDataRequestCmd(uint32_t parameter);
        void processStartDisplayAreaCmd(uint32_t parameter);
        void processHorizontalDisplayRangeCmd(uint32_t parameter);
        void processVerticalDisplayRangeCmd(uint32_t parameter);
        void processDisplayModeCmd(uint32_t parameter);
        void processSetVramSizeCmd(uint32_t parameter);
        void processReadGpuInternalRegCmd(uint32_t parameter);

        void updateRenderProjection();

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
                uint32_t texturePageColors : 2;
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

        RectanglePrimitiveData m_rectData{};

        GpuCommandsState m_commandState{};
        size_t m_remainingCmdArg{};
        size_t m_currentCmdParam{};
        static constexpr size_t MAX_COMMANDS_BUFFER_SIZE = 16;
        std::array<uint32_t, MAX_COMMANDS_BUFFER_SIZE> m_commandsFIFO{};

        std::vector<uint16_t> m_vram{};

        Renderer m_renderer{};
    };
};
