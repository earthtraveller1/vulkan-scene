include(ExternalProject)

ExternalProject_Add(
    "GLFW_external_project"
    PREFIX "${CMAKE_BINARY_DIR}/deps/glfw"
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "3.3.8"
    GIT_SHALLOW True
    CMAKE_ARGS "-D" "CMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps/glfw"
)

add_library("glfw" INTERFACE IMPORTED)
add_dependencies("glfw" "GLFW_external_project")
target_include_directories("glfw" INTERFACE "${CMAKE_BINARY_DIR}/deps/glfw/include")
target_link_directories("glfw" INTERFACE "${CMAKE_BINARY_DIR}/deps/glfw/lib")
target_link_libraries("glfw" INTERFACE "glfw3")

