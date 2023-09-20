cmake_minimum_required(VERSION 3.13)

project(vulkan-scene)

set(CMAKE_CXX_STANDARD 20)

include(ExternalProject)

ExternalProject_Add(
    "glfw"
    PREFIX "${CMAKE_BINARY_DIR}/deps/glfw"
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "3.3.8"
    GIT_SHALLOW True
    CMAKE_ARGS 
        "-D" "CMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps/glfw"
        "-D" "GLFW_BUILD_TESTS=False"
        "-D" "GLFW_BUILD_EXAMPLES=False"
)

ExternalProject_Add(
    "stb"
    PREFIX "${CMAKE_BINARY_DIR}/deps/stb"
    GIT_REPOSITORY "https://github.com/nothings/stb.git"
    GIT_TAG "master"
    GIT_SHALLOW True
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Add(
    "kirho"
    PREFIX "${CMAKE_BINARY_DIR}/deps/kirho"
    GIT_REPOSITORY "https://github.com/earthtraveller1/kirho.git"
    GIT_TAG "main"
    GIT_SHALLOW True
    CMAKE_ARGS "-D" "CMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps/kirho"
)

function(add_custom_deps target)
    add_dependencies(${target} glfw kirho)
    target_link_directories(${target} PRIVATE "${CMAKE_BINARY_DIR}/deps/glfw/lib")
    target_link_libraries(${target} PRIVATE glfw3)
    target_include_directories(
        ${target} PRIVATE 
        "${CMAKE_BINARY_DIR}/deps/kirho/include" 
        "${CMAKE_BINARY_DIR}/deps/glfw/include"
        "${CMAKE_BINARY_DIR}/deps/stb/src/stb"
    )
endfunction()

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
add_custom_deps(vulkan-scene)

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
