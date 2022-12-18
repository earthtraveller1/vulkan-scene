import tools.generator as generator
import sys


def run(configuration: generator.Configuration, clang_tidy: bool):
    # macOS currently does not support Vulkan and this project is essentially based
    # around Vulkan so...
    if sys.platform.startswith("darwin"):
        print("Sorry, but macOS is not supported.")
        exit(-1)

    vulkan_scene = generator.Executable(
        name="vulkan-scene",
        sources=[
            "src/commands.c",
            "src/device.c",
            "src/framebuffer-manager.c",
            "src/graphics-pipeline.c",
            "src/main.c",
            "src/swap-chain.c",
            "src/utils.c",
            "src/vertex-buffer.c",
            "src/vk-ext.c"
        ],
        shader_sources=[
            "shaders/basic.vert",
            "shaders/basic.frag"
        ],
        configuration=configuration,
        include_directories=[
            "deps/Vulkan-Headers/build/install/include"
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

    vulkan_scene.generate(clang_tidy)


if __name__ == "__main__":
    configuration = generator.Configuration.DEBUG
    clang_tidy = False
    
    for arg in sys.argv:
        if arg == "release":
            configuration = generator.Configuration.RELEASE
        elif arg == "clang-tidy":
            clang_tidy = True
            
    if clang_tidy:
        print("Enabling clang-tidy static analyzer.")

    run(configuration, clang_tidy)
