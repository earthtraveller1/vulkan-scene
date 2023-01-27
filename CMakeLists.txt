cmake_minimum_required(VERSION 3.13)

project(vulkan-scene)

cmake_policy(SET CMP0111 NEW)

include("deps.cmake")

if (WIN32)
    set(VULKAN_LIBRARY_TYPE STATIC)
else()
    set(VULKAN_LIBRARY_TYPE SHARED)
endif()

add_library(VulkanLib IMPORTED ${VULKAN_LIBRARY_TYPE})
set_property(TARGET VulkanLib PROPERTY IMPORTED_LOCATION "${VULKAN_LIBRARY}")
target_include_directories(VulkanLib INTERFACE "${VULKAN_INCLUDE_DIR}")

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -pedantic)
endif()

add_executable(vulkan-scene)

target_link_libraries(vulkan-scene PRIVATE VulkanLib)

if (UNIX AND NOT APPLE)
    target_link_libraries(vulkan-scene PRIVATE xcb)
endif()

add_subdirectory(src)