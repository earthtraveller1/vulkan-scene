import sys
import os
import tools.msvc as msvc
import tools.cmake as cmake

WORKING_DIR = os.getcwd()

def build_vulkan_loader(configuration: str):
    if os.name == "nt":
        generator = None
    else:
        generator = "Ninja"
    
    cmake.build_and_install(
        source_dir = "deps/Vulkan-Headers",
        binary_dir = "deps/Vulkan-Headers/build",
        install_prefix = "deps/Vulkan-Headers/build/install",
        generator = generator,
        configuration = configuration
    )
    
    cmake.build_and_install(
        source_dir = "deps/Vulkan-Loader",
        binary_dir = "deps/Vulkan-Loader/build",
        install_prefix = "deps/Vulkan-Loader/build/install",
        variables = {
            # Needs to be an absolute path.
            "VULKAN_HEADERS_INSTALL_DIR": f"{WORKING_DIR}/deps/Vulkan-Headers/build/install"
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