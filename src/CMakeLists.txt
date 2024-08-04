add_executable(${PROJECT_NAME} main.cpp psx_system.cpp psx_system.h)

add_subdirectory(cpu)

find_package(SDL2 REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} cpu)    
