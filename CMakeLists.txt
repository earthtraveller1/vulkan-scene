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

if (MSVC)
    target_compile_options("vulkan-scene" PRIVATE "/W4")
    target_link_options("vulkan-scene" PRIVATE "/W4")
else()
    target_compile_options("vulkan-scene" PRIVATE "-Wall" PRIVATE "-pedantic")
    target_link_options("vulkan-scene" PRIVATE "-Wall" PRIVATE "-pedantic")
endif()

add_dependencies("vulkan-scene" "GLFW")

function(compile_shader shader)
    add_custom_command(
        OUTPUT "${shader}.spv"
        COMMAND "${Vulkan_GLSLC_EXECUTABLE}" 
        ARGS "-o" "${CMAKE_CURRENT_SOURCE_DIR}/${shader}.spv" "${CMAKE_CURRENT_SOURCE_DIR}/${shader}"
        MAIN_DEPENDENCY "${shader}"
    )
    
    target_sources("vulkan-scene" PRIVATE "${shader}")
endfunction()

compile_shader("shaders/basic.vert")
compile_shader("shaders/basic.frag")

target_link_libraries("vulkan-scene" PRIVATE "Vulkan::Vulkan" "glfw3")
target_link_directories("vulkan-scene" PRIVATE "${CMAKE_BINARY_DIR}/deps/glfw/lib")
target_include_directories("vulkan-scene" PRIVATE ${VULKAN_INCLUDE_DIR} "${CMAKE_BINARY_DIR}/deps/glfw/include")

if (WIN32)
    target_compile_definitions("vulkan-scene" PRIVATE "_CRT_SECURE_NO_WARNINGS")
endif()

if (UNIX)
    # These are libraries that are required by GLFW.
    target_link_libraries("vulkan-scene" PRIVATE "dl" "m" "X11")
endif()

add_subdirectory("src")
