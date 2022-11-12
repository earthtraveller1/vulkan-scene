import ninja
import os
import enum

if os.name == "nt":
    DEFAULT_COMPILER = "cl"
    DEFAULT_LINKER = "link"
    
    OBJECT_FILE_EXT = ".obj"
    EXECUTABLE_EXT = ".exe"
    
    BASIC_COMPILE_OPTIONS = "-W4 -c $in -Fo:$out"
    BASIC_LINK_OPTIONS = "$in -out:$out"
else:
    DEFAULT_COMPILER = "gcc"
    DEFAULT_LINKER = "gcc"
    
    OBJECT_FILE_EXT = ".o"
    EXECUTABLE_EXT = ""

    BASIC_COMPILE_OPTIONS = "-Wall -pedantic -c $in -o $out"
    BASIC_LINK_OPTIONS = "$in -o $out"

class Executable:
    def __init__(self, name: str, sources: list, compiler: str = DEFAULT_COMPILER, linker: str = DEFAULT_LINKER, compile_options: str = "", linker_options: str = "", defer_generation: bool = False):
        self.name = name
        self.sources = sources
        self.compiler = compiler
        self.linker = linker
        self.compile_options = f"{compile_options} {BASIC_COMPILE_OPTIONS}"
        self.link_options = f"{linker_options} {BASIC_LINK_OPTIONS}"
        
        if not defer_generation:
            self.generate()
    
    def generate(self):
        output_file = open("build.ninja", "w")
        writer = ninja.Writer(output_file, 80)
        
        writer.rule("cc", f"{self.compiler} {self.compile_options}")
        writer.rule("ln", f"{self.linker} {self.link_options}")
        
        objects = []
        for source in self.sources:
            object = f"objects/{source}{OBJECT_FILE_EXT}"
            
            # Check if all the parent directories required to put the object f-
            # ile in exists, and make them if they don't.
            object_dir = os.path.dirname(object)
            if not os.path.isdir(object_dir):
                os.makedirs(object_dir)
            
            writer.build(object, "cc", source)
            objects.append(object) # We need to store the objects so that we c-
                                   # an build the executable later on.
        
        writer.build(f"{self.name}{EXECUTABLE_EXT}", "ln", objects)
        writer.close()