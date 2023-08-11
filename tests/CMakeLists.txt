include_directories("${CMAKE_SOURCE_DIR}/src")
link_libraries(kirho glfw Vulkan::Vulkan)

add_executable(window window.cpp ../src/window.cpp)
add_test(NAME window COMMAND window)
target_precompile_headers(window PRIVATE ../src/pch.hpp)

add_executable(instance instance.cpp ../src/device.cpp)
add_test(NAME instance COMMAND instance)
target_precompile_headers(instance PRIVATE ../src/pch.hpp)
