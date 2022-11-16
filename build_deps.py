import os
import sys
import shutil

def run(configuration: str):
    configure_command = f"cmake -S deps/Vulkan-Loader -B deps/Vulkan-Loader/build -D UPDATE_DEPS=true"
    
    if os.name != "nt":
        configure_command += "-D CMAKE_BUILD_TYPE={configuration} -G Ninja"
    
    build_command = f"cmake --build deps/Vulkan-Loader/build"
    install_command = f"cmake --install deps/Vulkan-Loader/build --prefix deps/Vulkan-Loader/build/install --config {configuration}"
    
    os.system(configure_command)
    os.system(build_command)
    os.system(install_command)

if __name__ == "__main__":
    if len(sys.argv) > 1:
        configuration = sys.argv[1]
    else:
        configuration = "Debug"
    
    run(configuration)