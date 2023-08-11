include_directories("${CMAKE_SOURCE_DIR}/src")
link_libraries(kirho glfw Vulkan::Vulkan)

add_executable(window window.cpp ../src/window.cpp)
add_test(NAME window COMMAND window)
target_precompile_headers(window PUBLIC ../src/pch.hpp)

add_executable(instance instance.cpp ../src/device.cpp)
add_test(NAME instance COMMAND instance)
target_precompile_headers(instance PRIVATE ../src/pch.hpp)

add_executable(devices devices.cpp ../src/device.cpp ../src/window.cpp)
add_test(NAME devices COMMAND devices)
target_precompile_headers(devices PRIVATE ../src/pch.hpp)
