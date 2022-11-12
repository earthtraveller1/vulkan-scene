import ninja
import os
import enum

SOURCE_ROOT = os.getcwd()

class TargetType(enum.Enum):
    EXECUTABLE = 0
    LIBRARY = 1

class Target:
    def __init__(self, name: str, sources: list = [], type: TargetType = TargetType.EXECUTABLE):
        self.name = name
        self.sources = sources
        self.type = type
    
    def generate(self, writer: ninja.Writer, compiler: str, linker: str):
        writer.rule(name=f"{self.name}c", command=f"{compiler} -c $in -Fo:$out")
        writer.rule(name=f"{self.name}l", command=f"{linker} $in -out:$out")
        
        objects = []
        for source in self.sources:
            object = f"objects/{source}.obj"
            objects.append(object)
            if not os.path.isdir(os.path.dirname(object)):
                os.makedirs(os.path.dirname(object))
        
        for i in range(0, len(self.sources)):
            writer.build(objects[i], f"{self.name}c", self.sources[i])
        
        writer.build(f"{self.name}.exe", f"{self.name}l", objects)

class Project:
    def __init__(self, targets: list = []):
        self.targets = targets
    
    def generate(self, compiler: str, linker: str):
        writer = ninja.Writer(open("build.ninja", "w"), 80)
        
        for target in self.targets:
            target.generate(writer, compiler, linker)