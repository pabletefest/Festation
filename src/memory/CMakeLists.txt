add_library(memory "")

target_sources(memory
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/virtual_mem_allocator_utils.cpp
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/virtual_mem_allocator_utils.h
   )
target_include_directories(memory
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
  )