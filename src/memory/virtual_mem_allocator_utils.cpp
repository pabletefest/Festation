// SUPPORTING UNIX-BASED OS ONLY FOR NOW (Windows is planned)

#include "virtual_mem_allocator_utils.hpp"

#if defined(__linux__) || defined(__unix__) || defined(__FreeBSD__) || defined(__APPLE__)
    #include <sys/mman.h>
#endif

#include <stdio.h>

static constexpr const uint32_t MAIN_RAM_SIZE = 2 * 1024 * 1024;
static constexpr const uint32_t GPU_VRAM_SIZE = 1 * 1024 * 1024;

uint8_t *festation::allocVirtMemForMainRAM()
{
    uint8_t* virtMemBuffer = nullptr;

#if defined(__linux__) || defined(__unix__) || defined(__FreeBSD__) || defined(__APPLE__)
        virtMemBuffer = (uint8_t*)mmap(NULL, MAIN_RAM_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif

    if (virtMemBuffer)
        printf("- 2MB of virtual memory allocated for PSX main RAM\n");

    return virtMemBuffer;
}

int festation::deallocVirtMemForMainRAM(void *mainMemory)
{
    if (!mainMemory)
    {
        printf("Can't deallocate main RAM buffer! Null pointer given so it's assumed it's already been deallocated!\n");
        return 0;
    }

#if defined(__linux__) || defined(__unix__) || defined(__FreeBSD__) || defined(__APPLE__)
    return munmap(mainMemory, MAIN_RAM_SIZE);
#endif
}

uint8_t *festation::allocVirtMemForGPUVRAM()
{
    uint8_t* virtMemBuffer = nullptr;
#if defined(__linux__) || defined(__unix__) || defined(__FreeBSD__) || defined(__APPLE__)
        virtMemBuffer = (uint8_t*)mmap(NULL, GPU_VRAM_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif

    if (virtMemBuffer)
        printf("- 1MB of virtual memory allocated for PSX GPU VRAM\n");

    return virtMemBuffer;
}

int festation::deallocVirtMemForGPUVRAM(void *gpuMemory)
{
    if (!gpuMemory)
    {
        printf("Can't deallocate GPU VRAM buffer! Null pointer given so it's assumed it's already been deallocated!\n");
        return 0;
    }

#if defined(__linux__) || defined(__unix__) || defined(__FreeBSD__) || defined(__APPLE__)
    return munmap(gpuMemory, GPU_VRAM_SIZE);
#endif
}
