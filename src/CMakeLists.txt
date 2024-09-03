set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(cpu)
add_subdirectory(kernel_bios)
add_subdirectory(memory)

add_executable(${PROJECT_NAME} main.cpp 
                               psx_system.cpp 
                               psx_system.hpp
                               cpu/psx_cw33300_cpu.cpp 
                               cpu/psx_cw33300_cpu.hpp 
                               cpu/cpu_masks_types_utils.hpp 
                               cpu/psx_cpu_state.hpp 
                               cpu/mips_r3000a_opcodes.cpp 
                               cpu/mips_r3000a_opcodes.hpp
                               cpu/coprocessor_cp0_opcodes.cpp
                               cpu/coprocessor_cp0_opcodes.hpp
                               cpu/exceptions_handling.cpp
                               cpu/exceptions_handling.hpp
                               kernel_bios/bios.cpp
                               kernel_bios/tty.cpp
                               kernel_bios/bios.hpp
                               kernel_bios/tty.hpp
                               memory/virtual_mem_allocator_utils.cpp
                               memory/virtual_mem_allocator_utils.hpp
                               memory/memory_map_masks.hpp)


find_package(SDL2 REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} cpu kernel_bios memory)
