#include <cstdint>

namespace festation {
    class PsxGpu {
    public:
        PsxGpu();
        ~PsxGpu();

        void reset();

        uint32_t read32(uint32_t address);
        void write32(uint32_t address, uint32_t value);

    private:
        void parseCommandGP0(uint32_t commandWord);
        void processRectangleCommand(uint32_t commandWord);
        
        void parseCommandGP1(uint32_t commandWord);

    private:
        union {
            struct {

            };

            uint32_t raw;
        } GPUREAD;

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
                uint32_t interlaceFIeld : 1;
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
        } GPUSTAT;

        GpuCommandsState commandState;
        size_t currentCmdArg;
    };
};