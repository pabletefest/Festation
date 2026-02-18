find_package(OpenGL REQUIRED)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# add_subdirectory(cpu)
# add_subdirectory(kernel_bios)
# add_subdirectory(memory)

add_executable(${PROJECT_NAME} main.cpp)

# target_compile_definitions(${PROJECT_NAME} PUBLIC
#   "$<$<CONFIG:Debug>:DISASSEMBLY>"
# )

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/psx_system.cpp 

    ${CMAKE_CURRENT_SOURCE_DIR}/cpu/psx_cw33300_cpu.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu/mips_r3000a_opcodes.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu/coprocessor_cp0_opcodes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu/exceptions_handling.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/dma/dma_channel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dma/dma_control.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/gpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/renderer/renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/renderer/texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/renderer/hw/OpenGL/ogl_texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/renderer/shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/renderer/hw/OpenGL/ogl_shader.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/kernel_bios/bios.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel_bios/tty.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/memory/virtual_mem_allocator_utils.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/utils/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/file_reader.cpp
)

# find_package(SDL2 REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/src
    # ${PROJECT_SOURCE_DIR}/cpu
    # ${PROJECT_SOURCE_DIR}/dma
    # ${PROJECT_SOURCE_DIR}/gpu
    # ${PROJECT_SOURCE_DIR}/kernel_bios
    # ${PROJECT_SOURCE_DIR}/memory
    # ${PROJECT_SOURCE_DIR}/utils
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL glad glm::glm)
# target_link_libraries(${PROJECT_NAME} PRIVATE cpu kernel_bios memory)
