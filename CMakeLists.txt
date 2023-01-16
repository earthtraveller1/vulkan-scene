cmake_minimum_required(VERSION 3.13)

project(vulkan-scene)

include("deps.cmake")

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -pedantic)
endif()

add_executable(vulkan-scene)

target_include_directories(vulkan-scene PRIVATE "${VULKAN_INCLUDE_DIR}")
target_link_libraries(vulkan-scene PRIVATE "${VULKAN_LIBRARY}")

add_subdirectory(src)