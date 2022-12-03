import sys
import os
import tools.msvc as msvc
import tools.cmake as cmake

def build_vulkan_loader(configuration: str):
    if os.name == "nt":
        generator = None
    else:
        generator = "Ninja"
    
    cmake.build_and_install(
        source_dir = "deps/Vulkan-Loader",
        binary_dir = "deps/Vulkan-Loader/build",
        install_prefix = "deps/Vulkan-Loader/build/install",
        variables = {
            "UPDATE_DEPS": "ON"
        },
        generator = generator,
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