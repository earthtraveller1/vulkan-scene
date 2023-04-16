cmake_minimum_required(VERSION 3.13)

project("vulkan-scene" LANGUAGES "C")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package("Vulkan")

add_executable("vulkan-scene")

target_link_libraries("vulkan-scene" PRIVATE "Vulkan::Vulkan")
target_include_directories("vulkan-scene" PRIVATE ${VULKAN_INCLUDE_DIR})

add_subdirectory("src")
