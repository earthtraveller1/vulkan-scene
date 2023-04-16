cmake_minimum_required(VERSION 3.13)

project("vulkan-scene" LANGUAGES "C")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package("Vulkan")

include("ExternalProject")

ExternalProject_Add(
    "GLFW" 
    GIT_REPOSITORY "https://github.com/glfw/glfw" 
    INSTALL_DIR "${CMAKE_BINARY_DIR}/deps/glfw" 
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps/glfw"
)

add_executable("vulkan-scene")

target_link_libraries("vulkan-scene" PRIVATE "Vulkan::Vulkan" "glfw3")
target_link_directories("vulkan-scene" PRIVATE "${CMAKE_BINARY_DIR}/deps/glfw/lib")
target_include_directories("vulkan-scene" PRIVATE ${VULKAN_INCLUDE_DIR} "${CMAKE_BINARY_DIR}/deps/glfw/include")

add_subdirectory("src")
