cmake_minimum_required(VERSION 3.16)

project(main VERSION 1.0.0)

find_package(Vulkan REQUIRED FATAL_ERROR)

set(glm_DIR "vendor/glm/cmake/glm/")
find_package(glm REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(vendor/GLFW)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

include_directories(Engine/src)
file(GLOB_RECURSE SOURCES Engine/src/*.cpp Engine/src/*.h Engine/*.cpp Engine/*.h)

add_executable(main ${SOURCES})

target_link_libraries(main glfw)
target_link_libraries(main Vulkan::Vulkan)
target_link_libraries(main glm::glm)
