cmake_minimum_required(VERSION 3.13)

project(vulkan-scene)

include(ExternalProject)

ExternalProject_Add(
    Vulkan-Loader
    GIT_REPOSITORY "https://github.com/KhronosGroup/Vulkan-Loader"
    GIT_SHALLOW True
    PREFIX "${CMAKE_BINARY_DIR}/deps/Vulkan-Loader"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps/Vulkan-Loader"
               "-DUPDATE_DEPS=ON"
)

set(CMAKE_CXX_STANDARD 20)

add_executable(vulkan-scene)

target_include_directories(vulkan-scene PRIVATE
    "${CMAKE_BINARY_DIR}/deps/Vulkan-Loader/src/Vulkan-Loader/external/Vulkan-Headers/build/install/include"
)

target_link_directories(vulkan-scene PRIVATE
    "${CMAKE_BINARY_DIR}/deps/Vulkan-Loader/lib"
)

if (WIN32)
    target_link_libraries(vulkan-scene PRIVATE vulkan-1)
else()
    target_link_libraries(vulkan-scene PRIVATE vulkan xcb)
endif()

add_subdirectory(src)