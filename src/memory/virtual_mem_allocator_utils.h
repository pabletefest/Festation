#pragma once

#include <cstdint>

namespace festation
{
    uint8_t* allocateVirtMemForMainRAM(); 
    int deallocateVirtMemForMainRAM(void* mainMemory); 

    uint8_t* allocateVirtMemForGPUVRAM(); 
    int deallocateVirtMemForGPUVRAM(void* gpuMemory);
};
