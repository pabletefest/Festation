#pragma once
#include <cstdint>

namespace festation
{
    uint8_t* allocVirtMemForMainRAM(); 
    int deallocVirtMemForMainRAM(void* mainMemory); 

    uint8_t* allocVirtMemForGPUVRAM(); 
    int deallocVirtMemForGPUVRAM(void* gpuMemory);
};
