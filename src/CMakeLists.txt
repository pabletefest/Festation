set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(cpu)
add_subdirectory(memory)

add_executable(${PROJECT_NAME} main.cpp 
                               psx_system.cpp 
                               psx_system.h 
                               cpu/psx_cpu.cpp 
                               cpu/psx_cpu.h 
                               cpu/cpu_masks_types_utils.h 
                               cpu/mips_r3000a_state.h 
                               cpu/mips_r3000a_opcodes.cpp 
                               cpu/mips_r3000a_opcodes.h)


find_package(SDL2 REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} cpu)