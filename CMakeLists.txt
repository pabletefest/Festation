cmake_minimum_required(VERSION 3.5.0)
project(Festation VERSION 0.1.0 LANGUAGES C CXX)

# find_package(SDL2 REQUIRED)

# find_package(PkgConfig REQUIRED)
# pkg_check_modules(SDL2 REQUIRED sdl2)

# include_directories(${PROJECT_NAME} ${SDL2_INCLUDE_DIRS})

add_subdirectory(src)


# target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
# target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
# target_compile_options(${PROJECT_NAME} PUBLIC ${SDL2_CFLAGS_OTHER})

# target_include_directories(${PROJECT_NAME} PRIVATE inc/*)
# target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)
# target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})    
