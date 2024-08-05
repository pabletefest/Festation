# add_library(cpu 
#     psx_cpu.cpp 
#     psx_cpu.h)

add_library(cpu "")

target_sources(cpu
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/psx_cpu.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mips_r3000a_opcodes.cpp
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/psx_cpu.h
    ${CMAKE_CURRENT_LIST_DIR}/mips_r300a_state.h
    ${CMAKE_CURRENT_LIST_DIR}/mips_r3000a_opcodes.h
    ${CMAKE_CURRENT_LIST_DIR}/cpu_masks_utils.h
    )
target_include_directories(cpu
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
  )