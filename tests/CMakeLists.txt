cmake_policy(SET CMP0110 NEW)

include_directories("${CMAKE_SOURCE_DIR}/src")

add_executable(window window.cpp ../src/window.cpp)
add_test(NAME window COMMAND window)
add_custom_deps(window)
target_precompile_headers(window PUBLIC ../src/pch.hpp)

add_executable(instance instance.cpp ../src/device.cpp)
add_test(NAME instance COMMAND instance)
add_custom_deps(instance)
target_precompile_headers(instance PRIVATE ../src/pch.hpp)

add_executable(devices devices.cpp ../src/device.cpp ../src/window.cpp)
add_test(NAME "devices (no validation)" COMMAND devices)
add_custom_deps(devices)
target_precompile_headers(devices PRIVATE ../src/pch.hpp)

add_executable(devices-with-validation devices-with-validation.cpp
                                       ../src/device.cpp ../src/window.cpp)
add_custom_deps(devices-with-validation)
add_test(NAME "devices (with validation)" COMMAND devices-with-validation)
target_precompile_headers(devices-with-validation PRIVATE ../src/pch.hpp)

add_executable(
  swapchain swapchain.cpp ../src/swapchain.cpp ../src/device.cpp
            ../src/graphics.cpp ../src/window.cpp ../src/stb-image.cpp)
add_custom_deps(swapchain)
add_test(NAME "swapchain" COMMAND swapchain)
target_precompile_headers(swapchain PRIVATE ../src/pch.hpp)
