import tools.generator as generator
import sys


def run(configuration: generator.Configuration):
    # macOS currently does not support Vulkan and this project is essentially based
    # around Vulkan so...
    if sys.platform.startswith("darwin"):
        print("Sorry, but macOS is not supported.")
        exit(-1)

    vulkan_scene = generator.Executable(
        name="vulkan-scene",
        sources=[
            "src/device.c",
            "src/main.c",
            "src/vk-ext.c"
        ],
        configuration=configuration,
        include_directories=[
            "deps/Vulkan-Loader/external/Vulkan-Headers/build/install/include"
        ],
        link_directories=[
            "deps/Vulkan-Loader/build/install/lib"
        ],
        defer_generation=True
    )

    if sys.platform.startswith("win32"):
        vulkan_scene.add_source("src/platform/win32/window.c")
        vulkan_scene.link_libraries(["vulkan-1.lib", "user32.lib"])
    elif sys.platform.startswith("linux"):
        vulkan_scene.add_source("src/platform/x11/window.c")
        vulkan_scene.link_libraries(["vulkan", "xcb"])

    vulkan_scene.generate()


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "release":
        configuration = generator.Configuration.RELEASE
    else:
        configuration = generator.Configuration.DEBUG

    run(configuration)
