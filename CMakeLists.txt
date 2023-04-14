cmake_minimum_required(VERSION 3.13)

project("vulkan-scene" LANGUAGES "C")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable("vulkan-scene")

add_subdirectory("src")
