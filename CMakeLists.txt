cmake_minimum_required(VERSION 3.13)

project(vulkan-scene)

set(CMAKE_CXX_STANDARD 20)

include("deps/glfw.cmake")

find_package(Vulkan)

if(MSVC)
  # TODO
else()
  add_compile_options(-Wall -Wpedantic -Werror)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g3)
    add_link_options(-g3)
  endif()
endif()

add_executable(vulkan-scene)
target_link_libraries(vulkan-scene PRIVATE glfw)
target_include_directories(vulkan-scene PRIVATE deps/stb)

target_sources(vulkan-scene PRIVATE shaders/basic.vert.spv)

add_custom_command(
  OUTPUT ${CMAKE_SOURCE_DIR}/shaders/basic.vert.spv
  COMMAND glslc ARGS ${CMAKE_SOURCE_DIR}/shaders/basic.vert -o
          ${CMAKE_SOURCE_DIR}/shaders/basic.vert.spv
  MAIN_DEPENDENCY shaders/basic.vert)

add_custom_command(
  OUTPUT ${CMAKE_SOURCE_DIR}/shaders/basic.frag.spv
  COMMAND glslc ARGS ${CMAKE_SOURCE_DIR}/shaders/basic.frag -o
          ${CMAKE_SOURCE_DIR}/shaders/basic.frag.spv
  MAIN_DEPENDENCY shaders/basic.frag)

target_sources(vulkan-scene PRIVATE shaders/basic.vert.spv
                                    shaders/basic.frag.spv)

add_subdirectory(src)

include(CTest)
add_subdirectory(tests)
