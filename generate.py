import generator
import sys

# macOS currently does not support Vulkan and this project is essentially based
# around Vulkan so...
if sys.platform.startswith("darwin"):
    print("Sorry, but macOS is not supported.")
    exit(-1)

if sys.platform.startswith("win32"):
    VULKAN_LIB = "vulkan-1.lib"
    VULKAN_LINK_DIR = "deps/Vulkan-Loader/build/install/lib"
elif sys.platform.startswith("linux"):
    VULKAN_LINK_DIR = "deps/Vulkan-Loader/build/install/lib"
    VULKAN_LIB = "vulkan"


def run(configuration: generator.Configuration):
    generator.Executable(
        name="vulkan-scene",
        sources=[
            "src/main.c",
            "src/device.c"
        ],
        configuration=configuration,
        include_directories=[
            "deps/Vulkan-Loader/external/Vulkan-Headers/build/install/include"
        ],
        link_directories=[
            VULKAN_LINK_DIR
        ],
        link_libraries=[
            VULKAN_LIB
        ]
    )


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "release":
        configuration = generator.Configuration.RELEASE
    else:
        configuration = generator.Configuration.DEBUG

    run(configuration)
