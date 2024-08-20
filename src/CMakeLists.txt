set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(cpu)
add_subdirectory(memory)

add_executable(${PROJECT_NAME} main.cpp 
                               psx_system.cpp 
                               psx_system.hpp
                               cpu/psx_cpu.cpp 
                               cpu/psx_cpu.hpp 
                               cpu/cpu_masks_types_utils.hpp 
                               cpu/mips_r3000a_state.hpp 
                               cpu/mips_r3000a_opcodes.cpp 
                               cpu/mips_r3000a_opcodes.hpp
                               cpu/coprocessor_cp0_opcodes.cpp
                               cpu/coprocessor_cp0_opcodes.hpp
                               memory/virtual_mem_allocator_utils.cpp
                               memory/virtual_mem_allocator_utils.hpp)


find_package(SDL2 REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} cpu)