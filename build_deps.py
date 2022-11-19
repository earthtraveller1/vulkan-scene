import os
import sys
import shutil
import tools.cmake as cmake

def run(configuration: str):
    if os.name == "nt":
        generator = None
    else:
        generator = "Ninja"
    
    variables = {}
    if os.name != "nt":
        variables["CMAKE_BUILD_TYPE", configuration]
    
    cmake.configure("deps/Vulkan-Headers", "deps/Vulkan-Headers/build", variables, generator=generator)
    cmake.build("deps/Vulkan-Headers/build")
    cmake.install("deps/Vulkan-Headers/build", "deps/Vulkan-Headers/build/install", configuration)
    
    variables = {}
    variables["VULKAN_HEADERS_INSTALL_DIR"] = "deps/Vulkan-Headers/install"
    
    if os.name != "nt":
        variables["CMAKE_BUILD_TYPE", configuration]
    
    cmake.configure("deps/Vulkan-Loader", "deps/Vulkan-Loader/build", variables, generator)
    cmake.build("deps/Vulkan-Loader/build")
    cmake.install("deps/Vulkan-Loader/build/install")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        configuration = sys.argv[1]
    else:
        configuration = "Debug"
    
    run(configuration)