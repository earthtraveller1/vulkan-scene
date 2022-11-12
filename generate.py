import generator
import sys

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "debug":
        configuration = generator.Configuration.DEBUG
    else:
        configuration = generator.Configuration.RELEASE
    
    generator.Executable("vulkan-scene", [ "src/main.c" ], configuration=configuration)
