add_library(memory "")

target_sources(memory
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/virtual_mem_allocator_utils.cpp
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/virtual_mem_allocator_utils.hpp
    ${CMAKE_CURRENT_LIST_DIR}/memory_map_masks.hpp
   )
target_include_directories(memory
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
  )
