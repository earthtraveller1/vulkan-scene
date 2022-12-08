import build_deps
import generate
import os

def run():
    # Build all the dependencies
    build_deps.run()

    # Generate all the build scripts
    generate.run()
    
    # Actually build everything.
    os.system("ninja")

if __name__ == "__main__":
    run()