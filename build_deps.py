import os
import sys
import shutil
import tools.cmake as cmake

def build_vulkan_loader(configuration: str):
    if os.name == "nt":
        generator = None
    else:
        generator = "Ninja"
    
    project = cmake.Project(
        source_dir = "deps/Vulkan-Loader",
        binary_dir = "deps/Vulkan-Loader/build",
        variables = {
            "UPDATE_DEPS": "true",
            "CMAKE_BUILD_TYPE": configuration
        },
        generator = generator,
        configuration = configuration
    )
    
    project.configure()
    project.build()
    project.install("deps/Vulkan-Loader/build/install")

def run(configuration: str):
    build_vulkan_loader(configuration)

if __name__ == "__main__":
    if len(sys.argv) > 1:
        configuration = sys.argv[1]
    else:
        configuration = "Debug"
    
    run(configuration)