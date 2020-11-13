cmake_minimum_required(VERSION 3.16)

project(main VERSION 1.0.0)

find_package(Vulkan REQUIRED FATAL_ERROR)

set(glm_DIR "vendor/glm/cmake/glm/")
find_package(glm REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(vendor/GLFW)
add_subdirectory(vendor/spdlog)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

include_directories(Engine Engine/src)
file(GLOB_RECURSE SOURCES Engine/*.cpp Engine/*.h)

add_library(main ${SOURCES})

target_precompile_headers(main PRIVATE Engine/ashpch.h)

target_link_libraries(main glfw)
target_link_libraries(main Vulkan::Vulkan)
target_link_libraries(main glm::glm)
target_link_libraries(main spdlog)

file(GLOB_RECURSE GAME_SOURCES Game/*.cpp Game/*.h)
add_executable(game ${GAME_SOURCES})
target_link_libraries(game main)
