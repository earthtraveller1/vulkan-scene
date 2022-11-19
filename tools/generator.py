import tools.ninja as ninja
import os
import enum
import tools.msvc as msvc
import sys

if os.name == "nt":
    DEFAULT_COMPILER = "cl"
    DEFAULT_LINKER = "link"

    OBJECT_FILE_EXT = ".obj"
    EXECUTABLE_EXT = ".exe"

    BASIC_COMPILE_OPTIONS = "-W4 -FC -FS -c $in -Fo:$out"
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
        generate_script: str = "generate.py",
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
        self.generate_script = generate_script
        
        if os.name == "nt":
            msvc_location = msvc.find_msvc()
            
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

        self.compile_options = f"{compile_options} {BASIC_COMPILE_OPTIONS}"
        self.link_options = f" {BASIC_LINK_OPTIONS} {linker_options}"

        self.add_include_dirs(include_directories)
        self.add_library_directories(link_directories)
        self.link_libraries(link_libraries)

        # On Windows, we have to manually link some libraries and add some inc-
        # lude directories
        if os.name == "nt":
            windows_sdk = msvc.WindowsSDK()
            self.add_library_directories(
                [
                    f"\"{msvc_location}\\lib\\x64\"",
                    f"\"{windows_sdk.library_dir()}\\um\\x64\"",
                    f"\"{windows_sdk.library_dir()}\\ucrt\\x64\""
                ]
            )

            self.add_include_dirs(
                [
                    f"\"{windows_sdk.include_dir()}\\ucrt\"",
                    f"\"{windows_sdk.include_dir()}\\um\"",
                    f"\"{windows_sdk.include_dir()}\\ucrt\"",
                    f"\"{windows_sdk.include_dir()}\\cppwinrt\"",
                    f"\"{windows_sdk.include_dir()}\\shared\"",
                    f"\"{windows_sdk.include_dir()}\\winrt\"",
                    f"\"{msvc_location}\\include\""
                ]
            )

        if not defer_generation:
            self.generate()

    def add_source(self, source: str):
        self.sources.append(source)

    def add_include_dir(self, directory: str):
        self.compile_options += f" {INCLUDE_OPTION_PREFIX}{directory}"

    def add_include_dirs(self, directories: list):
        for directory in directories:
            self.add_include_dir(directory)

    def add_library_directory(self, link_dir: str):
        self.link_options += f" {LINK_DIRECTORY_PREFIX}{link_dir}"

    def add_library_directories(self, link_dirs: list):
        for dir in link_dirs:
            self.add_library_directory(dir)

    def link_library(self, library: str):
        self.link_options += f" {LINK_LIBRARY_PREFIX}{library}"

    def link_libraries(self, libraries: list):
        for library in libraries:
            self.link_library(library)

    def generate(self):
        output_file = open("build.ninja", "w")
        writer = ninja.Writer(output_file, 80)
        
        writer.rule("gen", f"{sys.executable} $in", generator=True)

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
        writer.build("build.ninja", "gen", self.generate_script)
        
        writer.close()
