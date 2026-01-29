#pragma once

namespace festation {
    enum Gpu0Commands {
        Misc                = 0b000,
        PolygonPrimitive    = 0b001,
        LinePrimitive       = 0b010,
        RectanglePrimitive  = 0b011,
        VramToVramBlit      = 0b100,
        CpuToVramBlit       = 0b101,
        VramToCpuBlit       = 0b110,
        Environment         = 0b111,
    };

    enum Gpu1Commands {
        ResetGpu                = 0x00,
        ResetCommandBuffer      = 0x01,
        AcknowledgeGpuInterrupt = 0x02,
        DisplayEnable           = 0x03,
        DmaDirectionDataRequest = 0x04,
        StartDisplayArea        = 0x05,
        HorizontalDisplayRange  = 0x06,
        VerticalDisplayRange    = 0x07,
        DisplayMode             = 0x08,
        SetVramSize             = 0x09,
        ReadGpuInternalRegister = 0x10,
    };

    enum class GpuCommandsState {
        WaitingForCommand,
        ProcessingRectangleCommand,
    };
};
