# A thin wrapper around CMake commands.

import os

class Project:
    def __init__(self, source_dir: str, binary_dir: str, variables: dict | None = None, generator: str | None = None, configuration: str | None = None):
        self.source_dir = source_dir
        self.binary_dir = binary_dir
        self.variables = variables
        self.generator = generator
        self.configuration = configuration
    
    def configure(self):
        command = f"cmake -S{self.source_dir} -B{self.binary_dir}"
        if self.generator != None:
            command += f" -G{self.generator}"
        
        if self.configuration != None:
            command += f" -DCMAKE_BUILD_TYPE={self.configuration}"
        
        if self.variables != None:
            for variable in self.variables.keys():
                command += f" -D{variable}={self.variables[variable]}"
        
        os.system(command)
    
    def build(self):
        command = f"cmake --build {self.binary_dir}"
        if self.configuration != None:
            command += f" --config {self.configuration}"
        
        os.system(command)
    
    def install(self, prefix):
        command = f"cmake --install {self.binary_dir} --prefix {prefix}"
        if self.configuration != None:
            command += f" --config {self.configuration}"
