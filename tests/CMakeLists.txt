include_directories("${CMAKE_SOURCE_DIR}/src")
link_libraries(kirho glfw)

add_executable(window window.cpp ../src/window.cpp)
add_test(NAME window COMMAND window)
target_precompile_headers(window PRIVATE ../src/pch.hpp)
