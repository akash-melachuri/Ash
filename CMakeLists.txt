cmake_minimum_required(VERSION 3.2)

function(add_shader TARGET SHADER)
    find_program(GLSLC glslc /usr/bin/)

    get_filename_component(OUTPUT ${SHADER} NAME)

    set(current-shader-path ${SHADER})
    set(current-output-path ${CMAKE_BINARY_DIR}/assets/shaders/${OUTPUT}.spv)

    # Add a custom command to compile GLSL to SPIR-V.
    get_filename_component(current-output-dir ${current-output-path} DIRECTORY)
    file(MAKE_DIRECTORY ${current-output-dir})

    add_custom_command(
           OUTPUT ${current-output-path}
           COMMAND ${GLSLC} -o ${current-output-path} ${current-shader-path}
           DEPENDS ${current-shader-path}
           IMPLICIT_DEPENDS CXX ${current-shader-path}
           VERBATIM)

    # Make sure our build depends on this output.
    set_source_files_properties(${current-output-path} PROPERTIES GENERATED TRUE)
    target_sources(${TARGET} PRIVATE ${current-output-path})
endfunction(add_shader)

project(ash VERSION 1.0.0)

if (DEFINED ENV{VULKAN_SDK})
	set(VULKAN_LIB "$ENV{VULKAN_SDK}/Lib")
	set(VULKAN_INCLUDE_DIR "$ENV{VULKAN_SDK}/Lib/INCLUDE")
endif()

find_package(Vulkan REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(ASSIMP_WARNINGS_AS_ERRORS OFF)

add_subdirectory(vendor/GLFW)
add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/assimp)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/VulkanMemoryAllocator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

include_directories(Engine Engine/src)
include_directories(vendor/entt/include)
include_directories(vendor/stb_image)

file(GLOB_RECURSE SOURCES Engine/*.cpp)
add_library(ash ${SOURCES})
target_include_directories(ash PUBLIC vendor/assimp/include)
target_precompile_headers(ash PRIVATE Engine/ashpch.h)
target_compile_options(ash PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

if (UNIX AND NOT APPLE)
    add_compile_definitions(ASH_LINUX)
endif()

if (WIN32)
    add_compile_definitions(ASH_WINDOWS)
endif()

target_link_libraries(ash glfw)
target_link_libraries(ash Vulkan::Vulkan)
target_link_libraries(ash glm::glm)
target_link_libraries(ash spdlog::spdlog)
target_link_libraries(ash assimp)
target_link_libraries(ash GPUOpen::VulkanMemoryAllocator)

file(GLOB_RECURSE GAME_SOURCES Game/*.cpp)
add_executable(game ${GAME_SOURCES})

file(GLOB_RECURSE SHADER_SOURCES ${CMAKE_SOURCE_DIR}/assets/shaders/*)
foreach(file ${SHADER_SOURCES})
    add_shader(game ${file})
endforeach()

target_compile_options(game PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)
target_link_libraries(game ash)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT game)

