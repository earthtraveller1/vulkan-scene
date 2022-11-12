import generator
import sys

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "release":
        configuration = generator.Configuration.RELEASE
    else:
        configuration = generator.Configuration.DEBUG
    
    generator.Executable("vulkan-scene", [ "src/main.c" ], configuration=configuration)
