import tools.ninja as ninja
import os
import enum
import tools.msvc as msvc

if os.name == "nt":
    DEFAULT_COMPILER = "cl"
    DEFAULT_LINKER = "link"

    OBJECT_FILE_EXT = ".obj"
    EXECUTABLE_EXT = ".exe"

    BASIC_COMPILE_OPTIONS = "-W4 -FS -c $in -Fo:$out"
    BASIC_LINK_OPTIONS = "$in -out:$out"
    
    COMPILE_DEBUG_OPTIONS = "-Zi -Od"
    COMPILE_RELEASE_OPTIONS = "-O2"
    
    LINK_DEBUG_OPTIONS = "-debug"
    LINK_RELEASE_OPTIONS = ""
    
    INCLUDE_OPTION_PREFIX = "-I"
    LINK_DIRECTORY_PREFIX = "-libpath:"
    LINK_LIBRARY_PREFIX = ""
else:
    DEFAULT_COMPILER = "gcc"
    DEFAULT_LINKER = "gcc"

    OBJECT_FILE_EXT = ".o"
    EXECUTABLE_EXT = ""

    BASIC_COMPILE_OPTIONS = "-Wall -pedantic -c $in -o $out"
    BASIC_LINK_OPTIONS = "$in -o $out"
    
    COMPILE_DEBUG_OPTIONS = "-g3 -Og"
    COMPILE_RELEASE_OPTIONS = "-O3"
    
    LINK_DEBUG_OPTIONS = "-g3 -Og"
    LINK_RELEASE_OPTIONS = "-O3"
    
    INCLUDE_OPTION_PREFIX = "-I"
    LINK_DIRECTORY_PREFIX = "-L"
    LINK_LIBRARY_PREFIX = "-l"

class Configuration(enum.Enum):
    DEBUG = 0
    RELEASE = 1

class Executable:
    def __init__(
        self, 
        name: str, 
        sources: list,
        configuration: Configuration = Configuration.DEBUG,
        compiler: str = DEFAULT_COMPILER, 
        linker: str = DEFAULT_LINKER, 
        compile_options: str = "", 
        linker_options: str = "", 
        include_directories: list = [],
        link_directories: list = [],
        link_libraries: list = [],
        defer_generation: bool = False
    ):
        self.name = name
        self.sources = sources
        
        msvc_location = msvc.find_msvc()
        
        if os.name == "nt":
            self.compiler = f"{msvc_location}\\bin\\Hostx64\\x64\\{DEFAULT_COMPILER}"
            self.linker = f"{msvc_location}\\bin\\Hostx64\\x64\\{DEFAULT_LINKER}"
        else:
            self.compiler = compiler
            self.linker = linker
        
        if configuration == Configuration.DEBUG:
            compile_options = f"{compile_options} {COMPILE_DEBUG_OPTIONS}"
            linker_options = f"{linker_options} {LINK_DEBUG_OPTIONS}"
        elif configuration == Configuration.RELEASE:
            compile_options = f"{compile_options} {COMPILE_RELEASE_OPTIONS}"
            linker_options = f"{linker_options} {LINK_RELEASE_OPTIONS}"
        
        for directory in include_directories:
            compile_options = f"{compile_options} {INCLUDE_OPTION_PREFIX}{directory}"
        
        self.compile_options = f"{compile_options} {BASIC_COMPILE_OPTIONS}"
        self.link_options = f" {BASIC_LINK_OPTIONS} {linker_options}"
        
        for directory in link_directories:
            self.add_library_directory(directory)
        
        for library in link_libraries:
            self.link_library(library)
        
        # On Windows, we have to manually link some libraries.
        windows_sdk = msvc.find_windows_sdk()
        if os.name == "nt":
            self.add_library_directory(f"\"{msvc_location}\\lib\\x64\"")
            self.add_library_directory(f"\"{windows_sdk}\\um\\x64\"")
            self.add_library_directory(f"\"{windows_sdk}\\ucrt\\x64\"")

        if not defer_generation:
            self.generate()
    
    def add_source(self, source: str):
        self.sources.append(source)
    
    def link_library(self, library: str):
        self.link_options += f" {LINK_LIBRARY_PREFIX}{library}"
    
    def add_library_directory(self, link_dir: str):
        self.link_options += f" {LINK_DIRECTORY_PREFIX}{link_dir}"
    
    def link_libraries(self, libraries: list):
        for library in libraries:
            self.link_library(library)

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
            # We need to store the objects so that we c-
            objects.append(object)
            # an build the executable later on.

        writer.build(f"{self.name}{EXECUTABLE_EXT}", "ln", objects)
        writer.close()
