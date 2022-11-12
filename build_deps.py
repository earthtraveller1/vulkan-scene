import os
import sys
import shutil

def run(configuration: str):
    if not os.path.isdir("deps"):
        os.makedirs("deps")
    
    if os.path.isdir("deps/Vulkan-Loader"):
        print("[ERROR]: deps/Vulkan-Loader already exists. Please remove it or do something.")
        return
    
    os.system("git clone --depth=1 https://github.com/KhronosGroup/Vulkan-Loader.git deps/Vulkan-Loader")
    
    os.system(f"cmake -S deps/Vulkan-Loader -B deps/Vulkan-Loader/build -D UPDATE_DEPS=true -D CMAKE_BUILD_TYPE={configuration} -G Ninja")
    os.system(f"cmake --build deps/Vulkan-Loader/build")
    os.system(f"cmake --install deps/Vulkan-Loader/build --prefix deps/Vulkan-Loader/build/install --config {configuration}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        configuration = sys.argv[1]
    else:
        configuration = "Debug"
    
    run(configuration)