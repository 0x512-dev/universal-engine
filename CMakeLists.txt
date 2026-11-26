cmake_minimum_required(VERSION 3.18)
project(UniversalEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2 sources are downloaded by the CI workflow; gradle passes the path.
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
add_subdirectory(${SDL2_SOURCE_PATH} SDL2-build)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)

# SDL's Java side loads a library named "main"
add_library(main SHARED ${CMAKE_SOURCE_DIR}/main.cpp ${ENGINE_SOURCES})

target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(main PRIVATE SDL2main SDL2 vulkan android log)
