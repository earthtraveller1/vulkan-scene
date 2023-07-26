cmake_minimum_required(VERSION 3.13)

project(vulkan-scene)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan)

if (MSVC)
    # TODO
else()
    add_compile_options(-Wall -Wpedantic -Werror)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g3)
        add_link_options(-g3)
    endif()
endif()

add_executable(vulkan-scene)
target_link_libraries(vulkan-scene PRIVATE Vulkan::Vulkan glfw)

add_subdirectory(src)
