# add_library(cpu 
#     psx_cpu.cpp 
#     psx_cpu.h)

add_library(cpu "")

target_sources(cpu
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/psx_cw33300_cpu.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mips_r3000a_opcodes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/coprocessor_cp0_opcodes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/exceptions_handling.cpp
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/psx_cw33300_cpu.hpp
    ${CMAKE_CURRENT_LIST_DIR}/psx_cpu_state.hpp
    ${CMAKE_CURRENT_LIST_DIR}/mips_r3000a_opcodes.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cpu_masks_types_utils.hpp
    ${CMAKE_CURRENT_LIST_DIR}/coprocessor_cp0_opcodes.hpp
    ${CMAKE_CURRENT_LIST_DIR}/exceptions_handling.hpp
  )
target_include_directories(cpu
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${CMAKE_CURRENT_LIST_DIR}/../memory
  )