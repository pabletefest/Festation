#include <cstdint>

namespace festation {
    class PsxGpu {
    public:
        PsxGpu();
        ~PsxGpu();

        uint32_t read32(uint32_t address);
        void write32(uint32_t address, uint32_t value);

    private:
        union {
            struct {

            };

            uint32_t raw;
        } GPUREAD;

        union {
            struct {

            };
            
            uint32_t raw;
        } GPUSTAT;
    };
};