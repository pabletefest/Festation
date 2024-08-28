add_library(kernel_bios "")

target_sources(kernel_bios
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/bios.cpp
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/bios.hpp
   )
target_include_directories(kernel_bios
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
  )