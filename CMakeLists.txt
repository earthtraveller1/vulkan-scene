cmake_minimum_required(VERSION 3.13)

project(vulkan-scene)

cmake_policy(SET CMP0111 NEW)

find_package(Vulkan)

function(compile_shader target shader_file)
    add_custom_command(
        OUTPUT "${CMAKE_SOURCE_DIR}/${shader_file}.spv"
        COMMAND glslc ${shader_file} -o ${shader_file}.spv
        MAIN_DEPENDENCY "${CMAKE_SOURCE_DIR}/${shader_file}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
    
    target_sources(${target} PRIVATE ${shader_file})
endfunction(compile_shader)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -pedantic)
endif()

add_executable(vulkan-scene)

compile_shader(vulkan-scene shaders/basic.vert)
compile_shader(vulkan-scene shaders/basic.frag)

target_link_libraries(vulkan-scene PRIVATE Vulkan::Vulkan)

if (UNIX AND NOT APPLE)
    target_link_libraries(vulkan-scene PRIVATE xcb)
endif()

add_subdirectory(src)