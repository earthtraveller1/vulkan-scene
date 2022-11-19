# A thin wrapper around CMake commands.

import os

def configure(source_dir: str, binary_dir: str, variables: dict):
    command = f"cmake -S{source_dir} -B{binary_dir}"
    for variable in variables.keys():
        command += f" -D{variable}={variables[variable]}"
    
    print(command) # Just printing out to see if it works.

def build(binary_dir: str):
    command = f"cmake --build {binary_dir}"
    print(command)

def install(build_dir: str, install_dir: str | None = None, config: str | None = None):
    command = f"cmake --install {build_dir}"
    if install_dir != None:
        command += f" --prefix {install_dir}"
    if config != None:
        command += f" --config {config}"
    print(command)