# A thin wrapper around CMake commands.

import subprocess

class Project:
    def __init__(self, source_dir: str, binary_dir: str, variables: dict | None = None, generator: str | None = None, configuration: str | None = None):
        self.source_dir = source_dir
        self.binary_dir = binary_dir
        self.variables = variables
        self.generator = generator
        self.configuration = configuration
    
    def configure(self):
        command = [ 'cmake', f'-S{self.source_dir}', f'-B{self.binary_dir}' ]
        if self.generator != None:
            command.append(f'-G{self.generator}')
        
        if self.configuration != None:
            command.append(f'-DCMAKE_BUILD_TYPE={self.configuration}')
        
        if self.variables != None:
            for variable in self.variables.keys():
                command.append(f'-D{variable}={self.variables[variable]}')
        
        subprocess.run(command, check=True)
    
    def build(self):
        command = [ 'cmake', '--build', self.binary_dir ]
        if self.configuration != None:
            command.append('--config')
            command.append(self.configuration)
        
        subprocess.run(command, check=True)
    
    def install(self, prefix):
        command = [ 'cmake', '--install', self.binary_dir, '--prefix', prefix ]
        if self.configuration != None:
            command.append('--config')
            command.append(self.configuration)
        
        subprocess.run(command, check=True)

def build_and_install(
    self, 
    source_dir: str, 
    binary_dir: str, 
    install_prefix: str,
    variables: dict | None = None, 
    generator: str | None = None, 
    configuration: str | None = None
):
    project = Project(source_dir, binary_dir, variables, generator, configuration)
    project.configure()
    project.build()
    project.install(install_prefix)
