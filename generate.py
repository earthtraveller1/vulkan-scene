import generator
import sys

def run(configuration: generator.Configuration):
    generator.Executable("vulkan-scene", [ "src/main.c" ], configuration=configuration)

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "release":
        configuration = generator.Configuration.RELEASE
    else:
        configuration = generator.Configuration.DEBUG
    
    run(configuration)
