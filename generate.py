import generator
import sys
import os

VULKAN_INCLUDE_DIR = "deps/Vulkan-Loader/external/Vulkan-Headers/build/install/include"

if os.name == "nt":
    VULKAN_LIB = "vulkan-1"
    VULKAN_LINK_DIR = "deps/Vulkan-Loader/build/install/lib"

def run(configuration: generator.Configuration):
    generator.Executable("vulkan-scene", [ "src/main.c" ], configuration=configuration)

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "release":
        configuration = generator.Configuration.RELEASE
    else:
        configuration = generator.Configuration.DEBUG
    
    run(configuration)
