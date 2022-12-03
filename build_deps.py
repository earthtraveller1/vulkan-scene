import os
import sys
import shutil
import tools.cmake as cmake

def build_vulkan_loader(configuration: str):
    cmake.build_and_install(
        source_dir = "deps/Vulkan-Loader",
        binary_dir = "deps/Vulkan-Loader/build",
        install_prefix = "deps/Vulkan-Loader/build/install",
        variables = {},
        generator = "Ninja",
        configuration = configuration,
    )

def run(configuration: str):
    build_vulkan_loader(configuration)

if __name__ == "__main__":
    if len(sys.argv) > 1:
        configuration = sys.argv[1]
    else:
        configuration = "Debug"
    
    run(configuration)